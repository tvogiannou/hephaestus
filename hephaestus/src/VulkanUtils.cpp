#include <hephaestus/VulkanUtils.h>

#include <hephaestus/Log.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/VulkanDispatcher.h>

#include <fstream>
#include <vector>


namespace hephaestus
{

// copy from common utils to avoid dependency
bool
s_GetBinaryFileContents(const char* filename, std::vector<char>& fileContents)
{
    std::ifstream file(filename, std::ios::binary);
    if (file.fail())
        return false;

    std::streampos begin = file.tellg();
    file.seekg(0, std::ios::end);
    std::streampos end = file.tellg();

    const size_t newSize = static_cast<size_t>(end - begin);
    fileContents.resize(newSize);

    file.seekg(0, std::ios::beg);
    file.read(fileContents.data(), end - begin);
    file.close();

    return true;
}

VulkanUtils::ShaderModuleHandle
VulkanUtils::CreateShaderModule(const VulkanDeviceManager& deviceManager, const char* filename)
{
    std::vector<char> code;
    if (s_GetBinaryFileContents(filename, code))
    {
        vk::ShaderModuleCreateInfo createInfo(
            vk::ShaderModuleCreateFlags(), code.size(), (uint32_t*)code.data());
        return deviceManager.GetDevice().createShaderModuleUnique(
            createInfo, nullptr);
    }
    else
        HEPHAESTUS_LOG_WARNING("Failed to open shader file: %s", filename);

    return ShaderModuleHandle();
}

bool 
VulkanUtils::CreateDepthImage(const VulkanDeviceManager& deviceManager, 
    uint32_t width, uint32_t height, ImageInfo& depthImageInfo)
{
    vk::Format format = vk::Format::eD32Sfloat;

    vk::ImageCreateInfo imageCreateInfo(
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        format,
        { (uint32_t)width, (uint32_t)height, 1 },	// extend 3D
        1,		// mip levels
        1,		// array layers
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::SharingMode::eExclusive,
        0, nullptr,		// queue families
        vk::ImageLayout::eUndefined
    );
    depthImageInfo.imageHandle = 
        deviceManager.GetDevice().createImageUnique(imageCreateInfo, nullptr);

    if (!VulkanUtils::AllocateImageMemory(
        deviceManager, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImageInfo))
        return false;

    deviceManager.GetDevice().bindImageMemory(
        depthImageInfo.imageHandle.get(), depthImageInfo.deviceMemory.get(), 0);

    vk::ImageViewCreateInfo viewCreateInfo(
        vk::ImageViewCreateFlags(),
        depthImageInfo.imageHandle.get(),
        vk::ImageViewType::e2D,
        format,
        {
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity
        },
        { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });	// sub resource range
    depthImageInfo.view = 
        deviceManager.GetDevice().createImageViewUnique(viewCreateInfo, nullptr);

    return true;
}

bool 
VulkanUtils::CreateBuffer(const VulkanDeviceManager& deviceManager, uint32_t size, 	
    vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memoryProperty,	BufferInfo& bufferInfo)
{
    bufferInfo.size = size;

    vk::BufferCreateInfo bufferCreateInfo(
        vk::BufferCreateFlags(),
        bufferInfo.size,
        usage,
        vk::SharingMode::eExclusive,
        0, nullptr);								// queue family indices

    bufferInfo.bufferHandle = deviceManager.GetDevice().createBufferUnique(
        bufferCreateInfo, nullptr);
    if (!VulkanUtils::AllocateBufferMemory(deviceManager, memoryProperty, bufferInfo))
        return false;

    deviceManager.GetDevice().bindBufferMemory(
        bufferInfo.bufferHandle.get(), bufferInfo.deviceMemory.get(), 0);

    return true;
}

bool 
VulkanUtils::AllocateBufferMemory(const VulkanDeviceManager& deviceManager, 
    vk::MemoryPropertyFlags requiredMemoryProperty, BufferInfo& bufferInfo)
{
    vk::MemoryRequirements bufferMemoryRequirements = 
        deviceManager.GetDevice().getBufferMemoryRequirements(
            bufferInfo.bufferHandle.get());
    vk::PhysicalDeviceMemoryProperties memoryProperties = 
        deviceManager.GetPhysicalDevice().getMemoryProperties();

    for (uint32_t memoryTypeIndex = 0; memoryTypeIndex < memoryProperties.memoryTypeCount; ++memoryTypeIndex)
    {
        if ((bufferMemoryRequirements.memoryTypeBits & (1 << memoryTypeIndex)) && 
            (memoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & requiredMemoryProperty))
        {
            vk::MemoryAllocateInfo allocateInfo(bufferMemoryRequirements.size, memoryTypeIndex);
            bufferInfo.deviceMemory = deviceManager.GetDevice().allocateMemoryUnique(
                allocateInfo, nullptr);
            return true;
        }
    }

    return false;
}

bool 
VulkanUtils::AllocateImageMemory(const VulkanDeviceManager& deviceManager, 
    vk::MemoryPropertyFlags requiredMemoryProperty, ImageInfo& imageInfo)
{
    vk::MemoryRequirements bufferMemoryRequirements =
        deviceManager.GetDevice().getImageMemoryRequirements(
            imageInfo.imageHandle.get());
    vk::PhysicalDeviceMemoryProperties memoryProperties =
        deviceManager.GetPhysicalDevice().getMemoryProperties();

    for (uint32_t memoryTypeIndex = 0; memoryTypeIndex < memoryProperties.memoryTypeCount; ++memoryTypeIndex)
    {
        if ((bufferMemoryRequirements.memoryTypeBits & (1 << memoryTypeIndex)) &&
            (memoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & requiredMemoryProperty))
        {
            vk::MemoryAllocateInfo allocateInfo(bufferMemoryRequirements.size, memoryTypeIndex);
            imageInfo.deviceMemory = deviceManager.GetDevice().allocateMemoryUnique(
                allocateInfo, nullptr);
            return true;
        }
    }

    return false;
}

bool
VulkanUtils::CreateImageTextureInfo(const VulkanDeviceManager& deviceManager, 
    uint32_t width, uint32_t height, ImageInfo &textureInfo)
{
    vk::ImageCreateInfo imageCreateInfo(
        vk::ImageCreateFlags(),
        vk::ImageType::e2D,
        vk::Format::eR8G8B8A8Unorm,
        { (uint32_t)width, (uint32_t)height, 1 },	// extend 3D
        1,		// mip levels
        1,		// array layers
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::SharingMode::eExclusive,
        0, nullptr,		// queue families
        vk::ImageLayout::eUndefined
    );
    textureInfo.imageHandle = 
        deviceManager.GetDevice().createImageUnique(imageCreateInfo, nullptr);

    // allocate memory for image
    if (!VulkanUtils::AllocateImageMemory(deviceManager, vk::MemoryPropertyFlagBits::eDeviceLocal, textureInfo))
        return false;

    deviceManager.GetDevice().bindImageMemory(
        textureInfo.imageHandle.get(), textureInfo.deviceMemory.get(), 0);

    vk::ImageViewCreateInfo viewCreateInfo(
        vk::ImageViewCreateFlags(),
        textureInfo.imageHandle.get(),
        vk::ImageViewType::e2D,
        vk::Format::eR8G8B8A8Unorm,
        {
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity
        },
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });	// sub resource range
    textureInfo.view = 
        deviceManager.GetDevice().createImageViewUnique(viewCreateInfo, nullptr);

    vk::SamplerCreateInfo samplerCreateInfo(
        vk::SamplerCreateFlags(),
        vk::Filter::eLinear, vk::Filter::eLinear,
        vk::SamplerMipmapMode::eNearest,
        vk::SamplerAddressMode::eClampToEdge,		// address mode U
        vk::SamplerAddressMode::eClampToEdge,		// V
        vk::SamplerAddressMode::eClampToEdge,		// W
        0.f,										// mip Lod bias
        VK_FALSE, 1.f,								// anisotropy
        VK_FALSE, vk::CompareOp::eAlways,			// compare
        0.f, 0.f,									// min/max Lod
        vk::BorderColor::eFloatTransparentBlack,
        VK_FALSE);									// un normalized coords
    textureInfo.sampler = deviceManager.GetDevice().createSamplerUnique(
        samplerCreateInfo, nullptr);

    return true;
}

bool 
VulkanUtils::CopyBufferDataStage(const VulkanDeviceManager& deviceManager, const BufferInfo &stageBufferInfo,
    const BufferUpdateInfo &updateInfo, const BufferInfo& dstBufferInfo, 
    vk::AccessFlagBits dstFinalAccessMask, vk::PipelineStageFlagBits dstStageMask, VkDeviceSize dstBufferOffset)
{
    HEPHAESTUS_ASSERT(stageBufferInfo.IsValid());
    HEPHAESTUS_ASSERT(dstBufferInfo.IsValid());
    HEPHAESTUS_ASSERT(updateInfo.dataSize <= dstBufferInfo.size);
    // TODO: assert dst buffer memory is device local?

    // copy vertex data to the stage buffer
    {
        void* stageBufferPtr = deviceManager.GetDevice().mapMemory(
            stageBufferInfo.deviceMemory.get(),
            0,				// offset
            updateInfo.dataSize,
            vk::MemoryMapFlags());
        if (stageBufferPtr == nullptr)
            return false;
        std::memcpy(stageBufferPtr, updateInfo.data, updateInfo.dataSize);

        vk::MappedMemoryRange flushRange = 
        { 
            stageBufferInfo.deviceMemory.get(), 
            0, 
            updateInfo.dataSize 
        };
        deviceManager.GetDevice().flushMappedMemoryRanges(flushRange);

        deviceManager.GetDevice().unmapMemory(stageBufferInfo.deviceMemory.get());
    }

    // use temporarily a command buffer to copy the data to the device local buffer
    {
        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        updateInfo.copyCmdBuffer.begin(beginInfo);

        vk::BufferCopy copyInfo(0, dstBufferOffset, updateInfo.dataSize);
        updateInfo.copyCmdBuffer.copyBuffer(
            stageBufferInfo.bufferHandle.get(),
            dstBufferInfo.bufferHandle.get(),
            copyInfo);

        // memory barrier to change access after the copy
        vk::BufferMemoryBarrier bufferMemoryBarrier(
            vk::AccessFlagBits::eMemoryWrite,
            dstFinalAccessMask,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            dstBufferInfo.bufferHandle.get(),
            0, VK_WHOLE_SIZE);
        updateInfo.copyCmdBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            dstStageMask,
            vk::DependencyFlags(),
            nullptr,
            bufferMemoryBarrier,
            nullptr);

        updateInfo.copyCmdBuffer.end();
    }

    // submit & wait for the queue now to finish the copy
    vk::SubmitInfo submitInfo(
        0, nullptr,
        nullptr,
        1, &updateInfo.copyCmdBuffer,
        0, nullptr);
    deviceManager.GetGraphicsQueueInfo().queue.submit(submitInfo, nullptr);

    deviceManager.GetDevice().waitIdle();

    return true;
}

bool
VulkanUtils::CopyImageDataStage(const VulkanDeviceManager& deviceManager, const BufferInfo &stageBufferInfo,
    const ImageInfo& imageInfo, const VulkanUtils::TextureUpdateInfo& textureUpdateInfo)
{
    HEPHAESTUS_ASSERT(stageBufferInfo.IsValid());
    HEPHAESTUS_ASSERT(textureUpdateInfo.dataSize <= stageBufferInfo.size);
    HEPHAESTUS_ASSERT(imageInfo.imageHandle);

    // copy image data to the stage buffer
    {
        void* stageBufferPtr = deviceManager.GetDevice().mapMemory(
            stageBufferInfo.deviceMemory.get(),
            0,	// offset
            textureUpdateInfo.dataSize,
            vk::MemoryMapFlags());
        if (stageBufferPtr == nullptr)
            return false;
        std::memcpy(stageBufferPtr, textureUpdateInfo.data, textureUpdateInfo.dataSize);

        vk::MappedMemoryRange flushRange = { stageBufferInfo.deviceMemory.get(), 0, VK_WHOLE_SIZE };
        deviceManager.GetDevice().flushMappedMemoryRanges(flushRange);
        deviceManager.GetDevice().unmapMemory(
            stageBufferInfo.deviceMemory.get());
    }

    // use temporarily a command buffer to write to the device image
    {
        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        textureUpdateInfo.copyCmdBuffer.begin(beginInfo);

        vk::ImageSubresourceRange imageSubresourceRange(
            vk::ImageAspectFlagBits::eColor,
            0, 1, 0, 1);

        {
            vk::ImageMemoryBarrier barrierFromUndefinedToTransferDst(
                vk::AccessFlags(),
                vk::AccessFlagBits::eTransferWrite,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eTransferDstOptimal,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                imageInfo.imageHandle.get(),
                imageSubresourceRange);
            textureUpdateInfo.copyCmdBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eTransfer,
                vk::DependencyFlags(),
                nullptr,
                nullptr,
                barrierFromUndefinedToTransferDst);
        }

        vk::BufferImageCopy copyInfo(
            0, 0, 0,											// buffer offset
            { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },		// subresource
            { 0, 0, 0 },										// image offset
            { textureUpdateInfo.width, textureUpdateInfo.height, 1 });		// image extent
        textureUpdateInfo.copyCmdBuffer.copyBufferToImage(
            stageBufferInfo.bufferHandle.get(),
            imageInfo.imageHandle.get(),
            vk::ImageLayout::eTransferDstOptimal,
            copyInfo);

        // memory barrier to change access after the copy
        {
            vk::ImageMemoryBarrier barrierFromTransferToShader(
                vk::AccessFlagBits::eTransferWrite,
                vk::AccessFlagBits::eShaderRead,
                vk::ImageLayout::eTransferDstOptimal,
                vk::ImageLayout::eShaderReadOnlyOptimal,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                imageInfo.imageHandle.get(),
                imageSubresourceRange);
            textureUpdateInfo.copyCmdBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eFragmentShader,
                vk::DependencyFlags(),
                nullptr,
                nullptr,
                barrierFromTransferToShader);
        }

        textureUpdateInfo.copyCmdBuffer.end();
    }

    // submit queue now to do the copy
    vk::SubmitInfo submitInfo(
        0, nullptr,
        nullptr,
        1, &textureUpdateInfo.copyCmdBuffer,
        0, nullptr);
    deviceManager.GetGraphicsQueueInfo().queue.submit(submitInfo, nullptr);

    deviceManager.GetDevice().waitIdle();

    return true;
}

bool 
VulkanUtils::CopyBufferDataHost(const VulkanDeviceManager& deviceManager,
    const BufferUpdateInfo &updateInfo, const BufferInfo& dstBufferInfo, VkDeviceSize dstBufferOffset)
{
    HEPHAESTUS_ASSERT(dstBufferInfo.IsValid());
    HEPHAESTUS_ASSERT(updateInfo.dataSize <= dstBufferInfo.size);

    void* mappedMemPtr = deviceManager.GetDevice().mapMemory(
        dstBufferInfo.deviceMemory.get(),
        dstBufferOffset,				// offset
        updateInfo.dataSize,
        vk::MemoryMapFlags());
    if (mappedMemPtr == nullptr)
        return false;
    std::memcpy(mappedMemPtr, updateInfo.data, updateInfo.dataSize);

    //vk::MappedMemoryRange flushRange = { dstBufferInfo.deviceMemory.get(), 0, VK_WHOLE_SIZE };
    vk::MappedMemoryRange flushRange = 
    { 
        dstBufferInfo.deviceMemory.get(), 
        dstBufferOffset, 
        updateInfo.dataSize 
    };
    deviceManager.GetDevice().flushMappedMemoryRanges(flushRange);

    deviceManager.GetDevice().unmapMemory(dstBufferInfo.deviceMemory.get());

    return true;
}

bool 
VulkanUtils::CreateDescriptorPool(const VulkanDeviceManager& deviceManager,
    uint32_t uniformSize, uint32_t combinedImgSamplerSize, DescriptorPoolHandle& descriptorPool)
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    poolSizes.emplace_back(vk::DescriptorType::eUniformBuffer, uniformSize);
    poolSizes.emplace_back(vk::DescriptorType::eCombinedImageSampler, combinedImgSamplerSize);

    vk::DescriptorPoolCreateInfo poolCreateInfo(
        vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 12u, 
        (uint32_t)poolSizes.size(), poolSizes.data());

    descriptorPool = deviceManager.GetDevice().createDescriptorPoolUnique(
        poolCreateInfo, nullptr);

    return true;
}

bool 
VulkanUtils::CreateRenderPass(const VulkanDeviceManager& deviceManager, 
    vk::Format format, vk::ImageLayout finalLayout, RenderPassHandle& renderPass)
{
    vk::AttachmentDescription colorAttachmentDescription(
        vk::AttachmentDescriptionFlags(),
        format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,		// initial layout
        finalLayout);	                    // final layout

    vk::AttachmentDescription depthAttachmentDescription(
        vk::AttachmentDescriptionFlags(),
        vk::Format::eD32Sfloat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eDontCare,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,						// initial layout
        vk::ImageLayout::eDepthStencilAttachmentOptimal);	// final layout


    vk::AttachmentReference colorAttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
    vk::AttachmentReference depthAttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::SubpassDescription subpassDescription(
        vk::SubpassDescriptionFlags(),
        vk::PipelineBindPoint::eGraphics,
        0, nullptr,
        1, &colorAttachmentReference,
        nullptr,						// resolve attachment
        &depthAttachmentReference,
        0, nullptr);

    std::vector<vk::SubpassDependency> dependencies;
    dependencies.push_back(vk::SubpassDependency(
        VK_SUBPASS_EXTERNAL,
        0,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::AccessFlagBits::eMemoryRead,
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::DependencyFlagBits::eByRegion));
    dependencies.push_back(vk::SubpassDependency(
        0,
        VK_SUBPASS_EXTERNAL,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::AccessFlagBits::eMemoryRead,
        vk::DependencyFlagBits::eByRegion));

    std::array<vk::AttachmentDescription, 2> attachments =
    { colorAttachmentDescription, depthAttachmentDescription };
    vk::RenderPassCreateInfo renderPassCreateInfo(
        vk::RenderPassCreateFlags(),
        (uint32_t)attachments.size(), attachments.data(),
        1, &subpassDescription,
        (uint32_t)dependencies.size(), dependencies.data());

    renderPass = deviceManager.GetDevice().createRenderPassUnique(
        renderPassCreateInfo, nullptr);

    return true;
}


uint32_t 
VulkanUtils::FixupFlushRange(const VulkanDeviceManager& deviceManager, uint32_t size)
{
    const vk::PhysicalDeviceProperties& properties = 
        deviceManager.GetPhysicalDevice().getProperties();
    const uint32_t atomSize = (uint32_t)properties.limits.nonCoherentAtomSize;

    return (size + atomSize - 1) & (~(atomSize - 1));
}

} // hephaestus