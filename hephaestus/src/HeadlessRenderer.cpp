#include <hephaestus/HeadlessRenderer.h>

#include <hephaestus/Log.h>
#include <hephaestus/PipelineBase.h>
#include <hephaestus/VulkanDispatcher.h>


namespace hephaestus
{

bool 
HeadlessRenderer::Init(const InitInfo& info)
{
    RendererBase::InitInfo baseInfo;
    {
        baseInfo.outputImageLayout = vk::ImageLayout::eTransferSrcOptimal;
        baseInfo.colorFormat = info.colorFormat;
    }
    if (!RendererBase::Init(baseInfo))
        return false;

    m_extent.setWidth(info.width);
    m_extent.setHeight(info.height);

    if (!VulkanUtils::CreateDepthImage(
        m_deviceManager, info.width, info.height, m_depthImageInfo))
        return false;

    // create frame image to render to
    {
        vk::ImageCreateInfo imageCreateInfo;
        imageCreateInfo.imageType = vk::ImageType::e2D;
        imageCreateInfo.format = info.colorFormat;
        imageCreateInfo.extent.width = info.width;
        imageCreateInfo.extent.height = info.height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
        imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
        imageCreateInfo.usage = 
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;

        HEPHAESTUS_CHECK_RESULT_HANDLE(m_frameImageInfo.imageHandle, 
            m_deviceManager.GetDevice().createImageUnique(imageCreateInfo, nullptr));

        // allocate memory for image
        if (!VulkanUtils::AllocateImageMemory(
            m_deviceManager, vk::MemoryPropertyFlagBits::eDeviceLocal, m_frameImageInfo))
            return false;

        m_deviceManager.GetDevice().bindImageMemory(
            m_frameImageInfo.imageHandle.get(), m_frameImageInfo.deviceMemory.get(), 0);

        vk::ImageViewCreateInfo viewCreateInfo;
        viewCreateInfo.viewType = vk::ImageViewType::e2D;
        viewCreateInfo.format = info.colorFormat;
        //viewCreateInfo.subresourceRange = {};
        viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;
        viewCreateInfo.image = m_frameImageInfo.imageHandle.get();

        HEPHAESTUS_CHECK_RESULT_HANDLE(m_frameImageInfo.view, 
            m_deviceManager.GetDevice().createImageViewUnique(viewCreateInfo, nullptr));
    }

    // create framebuffer
    {
        std::array<vk::ImageView, 2> attachments = {
            m_frameImageInfo.view.get(),
            m_depthImageInfo.view.get()
        };

        // setup the framebuffer for the currently rendered image
        vk::FramebufferCreateInfo frameBufferCreateInfo(
            vk::FramebufferCreateFlags(),
            m_renderPass.get(),
            2, attachments.data(),
            info.width,
            info.height,
            1);		// layers

        HEPHAESTUS_CHECK_RESULT_HANDLE(m_framebuffer, 
            m_deviceManager.GetDevice().createFramebufferUnique(frameBufferCreateInfo, nullptr));
    }

    // create destination image buffer
    {
        vk::ImageCreateInfo imageCreateInfo;
        imageCreateInfo.imageType = vk::ImageType::e2D;
        imageCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
        imageCreateInfo.extent.width = info.width;
        imageCreateInfo.extent.height = info.height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
        imageCreateInfo.tiling = vk::ImageTiling::eLinear;
        imageCreateInfo.usage = vk::ImageUsageFlagBits::eTransferDst;

        HEPHAESTUS_CHECK_RESULT_HANDLE(m_dstImageInfo.imageHandle, 
            m_deviceManager.GetDevice().createImageUnique(imageCreateInfo, nullptr));

        // allocate memory for image
        // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        if (!VulkanUtils::AllocateImageMemory(
            m_deviceManager, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, m_dstImageInfo))
            return false;

        m_deviceManager.GetDevice().bindImageMemory(
            m_dstImageInfo.imageHandle.get(), m_dstImageInfo.deviceMemory.get(), 0);
    }

    return true;
}

void 
HeadlessRenderer::Clear()
{
    HEPHAESTUS_LOG_ASSERT(m_deviceManager.GetDevice(), "No Vulkan device available");
    m_deviceManager.WaitDevice();

    m_framebuffer.reset(nullptr);
    m_dstImageInfo.Clear();
    m_frameImageInfo.Clear();

    RendererBase::Clear();
}

bool 
HeadlessRenderer::RenderBegin(VulkanUtils::FrameUpdateInfo& frameInfo) const
{
    m_deviceManager.WaitDevice();

    // set frame info
    {
        frameInfo.drawCmdBuffer = m_cmdBuffer.get();
        frameInfo.framebuffer = m_framebuffer.get();
        frameInfo.extent = m_extent;
        frameInfo.image = m_frameImageInfo.imageHandle.get();
        frameInfo.view = m_frameImageInfo.view.get();
        frameInfo.renderPass = m_renderPass.get();
    }

    // begin recording commands
    vk::CommandBufferBeginInfo cmdBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    frameInfo.drawCmdBuffer.begin(cmdBufferBeginInfo);

    // begin the render pass
    std::array<float, 4> colorClearValues = m_colorClearValues;
    std::array<vk::ClearValue, 2> clearValues = {};
    clearValues[0].color = colorClearValues;
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0u);
    vk::Rect2D renderArea = { {0, 0}, { frameInfo.extent } };
    vk::RenderPassBeginInfo renderPassBeginInfo(
        frameInfo.renderPass,
        frameInfo.framebuffer,
        renderArea,
        (uint32_t)clearValues.size(), clearValues.data());
    frameInfo.drawCmdBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    // update viewport and scissor
    {
        vk::Viewport viewport = {
            0.f, 0.f,
            (float)frameInfo.extent.width,
            (float)frameInfo.extent.height,
            0.f, 1.f
        };
        vk::Rect2D scissor = {
            { 0,0 },
            {
                frameInfo.extent.width,
                frameInfo.extent.height
            }
        };
        frameInfo.drawCmdBuffer.setViewport(0, viewport);
        frameInfo.drawCmdBuffer.setScissor(0, scissor);
    }

    return true;
}

bool 
HeadlessRenderer::RenderEnd(const VulkanUtils::FrameUpdateInfo& frameInfo) const
{
    frameInfo.drawCmdBuffer.endRenderPass();
    frameInfo.drawCmdBuffer.end();

    // submit graphics queue
    {
        vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
        VulkanUtils::FenceHandle fence;
        HEPHAESTUS_CHECK_RESULT_HANDLE(fence, 
            m_deviceManager.GetDevice().createFenceUnique(fenceCreateInfo, nullptr));
        m_deviceManager.GetDevice().resetFences(fence.get());

        //vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        vk::SubmitInfo submitInfo(
            0, nullptr,
            nullptr,
            1, &m_cmdBuffer.get(),
            0, nullptr);
        m_deviceManager.GetGraphicsQueueInfo().queue.submit(submitInfo, fence.get());
        //m_deviceManager.GetDevice().waitForFences(fence.get(), VK_TRUE, UINT64_MAX);
        if (m_deviceManager.GetDevice().waitForFences(
            fence.get(), VK_TRUE, UINT64_MAX) != vk::Result::eSuccess)
            return false;
    }

    return true;
}

void 
HeadlessRenderer::CopyRenderFrame() const
{
    // copy to the destination image
    // ref https://github.com/SaschaWillems/Vulkan/blob/master/examples/renderheadless/renderheadless.cpp

    m_deviceManager.WaitDevice();

    // Do the actual blit from the offscreen image to our host visible destination image
    vk::CommandBufferBeginInfo cmdBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    m_cmdBuffer.get().begin(cmdBufferBeginInfo);

    // Transition destination image to transfer destination layout
    {
        vk::ImageSubresourceRange imageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

        vk::ImageMemoryBarrier barrierFromLinearToTransfer(
            vk::AccessFlags(),
            vk::AccessFlagBits::eTransferWrite,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eTransferDstOptimal,
            m_deviceManager.GetGraphicsQueueInfo().familyIndex,
            m_deviceManager.GetGraphicsQueueInfo().familyIndex,
            m_dstImageInfo.imageHandle.get(),
            imageSubresourceRange);

        m_cmdBuffer.get().pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlags(),
            nullptr, nullptr, barrierFromLinearToTransfer);
    }

    // colorAttachment.image is already in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, and does not need to be transitioned

    vk::ImageCopy imageCopyRegion = {};
    imageCopyRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    imageCopyRegion.srcSubresource.layerCount = 1;
    imageCopyRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    imageCopyRegion.dstSubresource.layerCount = 1;
    imageCopyRegion.extent.width = m_extent.width;
    imageCopyRegion.extent.height = m_extent.height;
    imageCopyRegion.extent.depth = 1;

    m_cmdBuffer.get().copyImage(
        m_frameImageInfo.imageHandle.get(), vk::ImageLayout::eTransferSrcOptimal,
        m_dstImageInfo.imageHandle.get(), vk::ImageLayout::eTransferDstOptimal,
        imageCopyRegion);

    // Transition destination image to general layout, which is the required layout for mapping the image memory later on
    {
        vk::ImageSubresourceRange imageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

        vk::ImageMemoryBarrier barrierFromLinearToTransfer(
            vk::AccessFlagBits::eTransferWrite,
            vk::AccessFlagBits::eMemoryRead,
            vk::ImageLayout::eTransferDstOptimal,
            vk::ImageLayout::eGeneral,
            m_deviceManager.GetGraphicsQueueInfo().familyIndex,
            m_deviceManager.GetGraphicsQueueInfo().familyIndex,
            m_dstImageInfo.imageHandle.get(),
            imageSubresourceRange);

        m_cmdBuffer.get().pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlags(),
            nullptr, nullptr, barrierFromLinearToTransfer);
    }

    m_cmdBuffer.get().end();

    // submit & wait for the queue now to finish the copy
    vk::SubmitInfo submitInfo(
        0, nullptr,
        nullptr,
        1, &m_cmdBuffer.get(),
        0, nullptr);
    m_deviceManager.GetGraphicsQueueInfo().queue.submit(submitInfo, nullptr);

    m_deviceManager.GetDevice().waitIdle();
}

bool 
HeadlessRenderer::GetDstImageInfo(uint32_t& numChannels, uint32_t& width, uint32_t& height) const
{
    // TODO
    numChannels = 4u;
    width = m_extent.width;
    height = m_extent.height;

    return true;
}

bool 
HeadlessRenderer::GetDstImageData(char* data) const
{
    // Get layout of the image (including row pitch)
    vk::ImageSubresource subResource = {};
    subResource.aspectMask = vk::ImageAspectFlagBits::eColor;// VK_IMAGE_ASPECT_COLOR_BIT;
    vk::SubresourceLayout subResourceLayout;

    m_deviceManager.GetDevice().getImageSubresourceLayout(
        m_dstImageInfo.imageHandle.get(), &subResource, &subResourceLayout);

    // Map image memory so we can start copying from it
    const char* imagedata = nullptr;
    {
        void* temp = nullptr;
        HEPHAESTUS_CHECK_RESULT_RAW(temp, m_deviceManager.GetDevice().mapMemory(
                                        m_dstImageInfo.deviceMemory.get(), 0, VK_WHOLE_SIZE, vk::MemoryMapFlags()));

        imagedata = reinterpret_cast<const char*>(temp);
    }
    imagedata += subResourceLayout.offset;

    // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
    // Check if source is BGR and needs swizzle
    std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
    const bool colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), VK_FORMAT_R8G8B8A8_UNORM) != formatsBGR.end());

    // TODO
    if (colorSwizzle)
        return false;

    for (uint32_t y = 0; y < m_extent.height; y++)
    {
        // copy the full row using memcpy
        unsigned int *row = (unsigned int*)imagedata;
        std::memcpy(data, row, m_extent.width * 4u);
        data += m_extent.width * 4u;

        imagedata += subResourceLayout.rowPitch;
    }

    return true;
}

} // namespace hephaestus