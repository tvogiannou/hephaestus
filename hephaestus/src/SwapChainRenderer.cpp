#include <hephaestus/SwapChainRenderer.h>

#include <hephaestus/Log.h>



namespace hephaestus
{

bool 
SwapChainRenderer::Init(const InitInfo& info)
{
    HEPHAESTUS_LOG_ASSERT(m_deviceManager.GetDevice(), "No Vulkan device available");

    if (!CreateSwapChain())
        return false;

    // Need to initialize AFTER creating the swap chain since we are using the format from it
    RendererBase::InitInfo baseInfo;
    {
        baseInfo.outputImageLayout = vk::ImageLayout::ePresentSrcKHR;
        baseInfo.colorFormat = m_swapChainInfo.format;
    }
    if (!RendererBase::Init(baseInfo))
        return false;

    if (!CreateRenderingResources(info.numVirtualFrames))
        return false;

    return true;
}

void 
SwapChainRenderer::Clear()
{
    HEPHAESTUS_LOG_ASSERT(m_deviceManager.GetDevice(), "No Vulkan device available");
    m_deviceManager.WaitDevice();

    // buffers need to be destroyed before the graphics command pool
    for (VulkanUtils::VirtualFrameResources& frame : m_virtualFrames)
        frame.Clear();

    RendererBase::Clear();
}

SwapChainRenderer::RenderStatus 
SwapChainRenderer::RenderBegin(RenderInfo& renderInfo, RenderStats& stats)
{

    // update resource index
    HEPHAESTUS_LOG_ASSERT(m_nextAvailableVirtualFrameIndex < m_virtualFrames.size(), "Virtual frame index out of range");
    renderInfo.virtualFrameIndex = m_nextAvailableVirtualFrameIndex;
    VulkanUtils::VirtualFrameResources& currentResource = m_virtualFrames[m_nextAvailableVirtualFrameIndex];
    m_nextAvailableVirtualFrameIndex = (m_nextAvailableVirtualFrameIndex + 1) % m_virtualFrames.size();

    auto timer_waitStart = std::chrono::high_resolution_clock::now();

    // wait for previous commands to execute
    if (m_deviceManager.GetDevice().waitForFences(
        currentResource.fence.get(), VK_FALSE, 1000000000, m_dispatcher) != vk::Result::eSuccess)
        return RenderStatus::eRENDER_STATUS_FAIL_WAIT_DEVICE;
    m_deviceManager.GetDevice().resetFences(currentResource.fence.get(), m_dispatcher);

    auto timer_commandStart = std::chrono::high_resolution_clock::now();

    uint32_t imageIndex = UINT32_MAX;
    vk::Result result = m_deviceManager.GetDevice().acquireNextImageKHR(
        m_swapChainInfo.swapChainHandle.get(), UINT64_MAX,
        currentResource.imageAvailableSemaphore.get(), nullptr, &imageIndex, m_dispatcher);
    switch (result)
    {
    case vk::Result::eSuccess:
    case vk::Result::eSuboptimalKHR:
        break;
    case vk::Result::eErrorOutOfDateKHR:
    {
        if (!UpdateSwapChain())
            return RenderStatus::eRENDER_STATUS_FAIL_RESIZE;
        return RenderStatus::eRENDER_STATUS_RESIZE;
    }
    default:
        return RenderStatus::eRENDER_STATUS_FAIL_ACQUIRE_IMAGE;
    }

    if (imageIndex >= m_swapChainInfo.imageRefs.size())
        return RenderStatus::eRENDER_STATUS_FAIL_ACQUIRE_IMAGE;

    std::array<vk::ImageView, 2> attachments = {
        m_swapChainInfo.imageRefs[imageIndex].view.get(),
        m_depthImageInfo.view.get()
    };

    // setup the framebuffer for the currently rendered image
    vk::FramebufferCreateInfo frameBufferCreateInfo(
        vk::FramebufferCreateFlags(),
        m_renderPass.get(),
        2, attachments.data(),
        m_swapChainInfo.extent.width,
        m_swapChainInfo.extent.height,
        1);		// layers

    currentResource.framebuffer =
        m_deviceManager.GetDevice().createFramebufferUnique(frameBufferCreateInfo, nullptr, m_dispatcher);

    // set the render info
    {
        renderInfo.imageIndex = imageIndex;

        renderInfo.frameInfo.drawCmdBuffer = currentResource.commandBuffer.get();
        renderInfo.frameInfo.framebuffer = currentResource.framebuffer.get();
        renderInfo.frameInfo.extent = m_swapChainInfo.extent;
        renderInfo.frameInfo.image = m_swapChainInfo.imageRefs[imageIndex].image;
        renderInfo.frameInfo.view = m_swapChainInfo.imageRefs[imageIndex].view.get();
        renderInfo.frameInfo.renderPass = m_renderPass.get();
    }

    // begin recording commands
    vk::CommandBufferBeginInfo cmdBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    renderInfo.frameInfo.drawCmdBuffer.begin(cmdBufferBeginInfo, m_dispatcher);

    // add memory barrier to change from the present queue to the graphics queue 
    vk::ImageSubresourceRange imageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    if (m_deviceManager.GetPresentQueueInfo().familyIndex != m_deviceManager.GetGraphicsQueueInfo().familyIndex)
    {
        vk::ImageMemoryBarrier barrierFromPresentToClear(
            vk::AccessFlagBits::eMemoryRead,
            vk::AccessFlagBits::eMemoryRead,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::ePresentSrcKHR,
            m_deviceManager.GetPresentQueueInfo().familyIndex,
            m_deviceManager.GetGraphicsQueueInfo().familyIndex,
            renderInfo.frameInfo.image,
            imageSubresourceRange);

        renderInfo.frameInfo.drawCmdBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::DependencyFlags(),
            nullptr, nullptr, barrierFromPresentToClear, m_dispatcher);
    }

    // begin the render pass
    //std::array<float, 4> colorClearValues = { 0.7f, 0.88f, 0.9f, 1.0f };
    std::array<vk::ClearValue, 2> clearValues = {};
    clearValues[0].color = m_colorClearValues;
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0u);
    vk::Rect2D renderArea = { {0, 0}, { renderInfo.frameInfo.extent } };
    vk::RenderPassBeginInfo renderPassBeginInfo(
        renderInfo.frameInfo.renderPass,
        renderInfo.frameInfo.framebuffer,
        renderArea,
        (uint32_t)clearValues.size(), clearValues.data());
    renderInfo.frameInfo.drawCmdBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline, m_dispatcher);

    // update viewport and scissor
    {
        vk::Viewport viewport = {
            0.f, 0.f,
            (float)renderInfo.frameInfo.extent.width,
            (float)renderInfo.frameInfo.extent.height,
            0.f, 1.f
        };
        vk::Rect2D scissor = {
            { 0,0 },
            {
                renderInfo.frameInfo.extent.width,
                renderInfo.frameInfo.extent.height
            }
        };
        renderInfo.frameInfo.drawCmdBuffer.setViewport(0, viewport, m_dispatcher);
        renderInfo.frameInfo.drawCmdBuffer.setScissor(0, scissor, m_dispatcher);
    }

    stats.waitTime = std::chrono::duration<float, std::milli>(timer_commandStart - timer_waitStart).count();

    return RenderStatus::eRENDER_STATUS_COMPLETE;
}

SwapChainRenderer::RenderStatus
SwapChainRenderer::RenderEnd(const RenderInfo& renderInfo, RenderStats& stats)
{
    renderInfo.frameInfo.drawCmdBuffer.endRenderPass(m_dispatcher);

    // add memory barrier to change from the graphics queue to the present queue 
    vk::ImageSubresourceRange imageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    if (m_deviceManager.GetPresentQueueInfo().familyIndex != m_deviceManager.GetGraphicsQueueInfo().familyIndex)
    {
        vk::ImageMemoryBarrier barrierFromClearToPresent(
            vk::AccessFlagBits::eMemoryRead,
            vk::AccessFlagBits::eMemoryRead,
            vk::ImageLayout::ePresentSrcKHR,
            vk::ImageLayout::ePresentSrcKHR,
            m_deviceManager.GetGraphicsQueueInfo().familyIndex,
            m_deviceManager.GetPresentQueueInfo().familyIndex,
            renderInfo.frameInfo.image,
            imageSubresourceRange);

        renderInfo.frameInfo.drawCmdBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::DependencyFlags(),
            nullptr, nullptr, barrierFromClearToPresent, m_dispatcher);
    }

    renderInfo.frameInfo.drawCmdBuffer.end(m_dispatcher);

    auto timer_queueStart = std::chrono::high_resolution_clock::now();

    VulkanUtils::VirtualFrameResources& currentResource = m_virtualFrames[renderInfo.virtualFrameIndex];

    // submit graphics queue
    {
        vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        vk::SubmitInfo submitInfo(
            1, &currentResource.imageAvailableSemaphore.get(),
            &waitDstStageMask,
            1, &currentResource.commandBuffer.get(),
            1, &currentResource.finishedRenderingSemaphore.get());
        m_deviceManager.GetGraphicsQueueInfo().queue.submit(
            submitInfo, currentResource.fence.get(), m_dispatcher);
    }

    auto timer_presentStart = std::chrono::high_resolution_clock::now();

    // submit present queue
    {
        vk::PresentInfoKHR presentInfo(
            1, &currentResource.finishedRenderingSemaphore.get(),
            1, &m_swapChainInfo.swapChainHandle.get(),
            &renderInfo.imageIndex,
            nullptr);

        // Vulkan hpp throws an exception when VK_ERROR_OUT_OF_DATE_KHR is generated 
        // so we fall back to the C API for evaluating the result
        // Note that this seems to a problem due to GLFW not triggering the 
        // resize event *before* the queue present fails
        {
            VkPresentInfoKHR vkPresetInfo(presentInfo);
            // In FIFO, this is blocking potentially waiting for vsync to complete (instead of acquireNextImageKHR?)
            VkResult vkresult =
                m_dispatcher.vkQueuePresentKHR(VkQueue(m_deviceManager.GetPresentQueueInfo().queue), &vkPresetInfo);
            switch (vkresult)
            {
            case VK_SUCCESS:
                break;
            case VK_ERROR_OUT_OF_DATE_KHR:
            case VK_SUBOPTIMAL_KHR:
            {
                if (!UpdateSwapChain())
                    return RenderStatus::eRENDER_STATUS_FAIL_RESIZE;
                return RenderStatus::eRENDER_STATUS_RESIZE;
            }
            default:
                return RenderStatus::eRENDER_STATUS_FAIL_PRESENT;
            }
        }
    }

    auto timer_drawEnd = std::chrono::high_resolution_clock::now();

    // setup timers
    stats.queueTime = std::chrono::duration<float, std::milli>(timer_presentStart - timer_queueStart).count();
    stats.presentTime = std::chrono::duration<float, std::milli>(timer_drawEnd - timer_presentStart).count();

    return RenderStatus::eRENDER_STATUS_COMPLETE;
}

bool 
SwapChainRenderer::CreateRenderingResources(uint32_t numVirtualFrames)
{
    if (numVirtualFrames == 0)
        return false;

    m_virtualFrames.clear();
    m_virtualFrames.resize(numVirtualFrames);

    // set all resources
    for (VulkanUtils::VirtualFrameResources& resources : m_virtualFrames)
    {
        vk::CommandBufferAllocateInfo cmdBufferAllocateInfo(
            m_graphicsCommandPool.get(), vk::CommandBufferLevel::ePrimary, 1);
        std::vector<vk::CommandBuffer> buffer = m_deviceManager.GetDevice().allocateCommandBuffers(
            cmdBufferAllocateInfo, m_dispatcher);
        vk::PoolFree<vk::Device, vk::CommandPool, VulkanDispatcher> deleter(
            m_deviceManager.GetDevice(), m_graphicsCommandPool.get(), m_dispatcher);
        resources.commandBuffer = VulkanUtils::CommandBufferHandle(buffer.front(), deleter);

        vk::SemaphoreCreateInfo semaphoreCreateInfo;
        resources.finishedRenderingSemaphore = 
            m_deviceManager.GetDevice().createSemaphoreUnique(semaphoreCreateInfo, nullptr, m_dispatcher);
        resources.imageAvailableSemaphore = 
            m_deviceManager.GetDevice().createSemaphoreUnique(semaphoreCreateInfo, nullptr, m_dispatcher);

        vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
        resources.fence = m_deviceManager.GetDevice().createFenceUnique(fenceCreateInfo, nullptr, m_dispatcher);
    }

    return true;
}

bool
SwapChainRenderer::CreateSwapChain()
{
    m_canDraw = false;

    m_deviceManager.GetDevice().waitIdle(m_dispatcher);

    // clear swap chain images
    m_swapChainInfo.imageRefs.clear();

    // set format
    vk::SurfaceFormatKHR desiredFormat = { vk::Format::eUndefined, vk::ColorSpaceKHR::eSrgbNonlinear };
    {
        const std::vector<vk::SurfaceFormatKHR>& surfaceFormats =
            m_deviceManager.GetPhysicalDevice().getSurfaceFormatsKHR(
                m_deviceManager.GetPresentSurface(), m_dispatcher);
        HEPHAESTUS_LOG_ASSERT(!surfaceFormats.empty(), "Failed to find valid KHR surface format");
        desiredFormat = surfaceFormats[0];
        {
            if ((surfaceFormats.size() == 1) && (surfaceFormats[0].format == vk::Format::eUndefined))
                desiredFormat = { vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
            else
            {
                // Check if list contains most widely used R8 G8 B8 A8 format
                // with nonlinear color space
                for (vk::SurfaceFormatKHR surfaceFormat : surfaceFormats)
                {
                    if (surfaceFormat.format == vk::Format::eR8G8B8A8Unorm)
                    {
                        desiredFormat = surfaceFormat;
                        break;
                    }
                }
            }
        }
    }

    // set surface properties
    uint32_t desiredImageCount = 0u;
    vk::Extent2D desiredExtent;
    vk::ImageUsageFlags desiredUsage = (vk::ImageUsageFlagBits)(-1);
    vk::SurfaceTransformFlagBitsKHR desiredTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
    {
        vk::SurfaceCapabilitiesKHR surfaceCapabilities =
            m_deviceManager.GetPhysicalDevice().getSurfaceCapabilitiesKHR(m_deviceManager.GetPresentSurface(), m_dispatcher);
        desiredImageCount =
            (surfaceCapabilities.maxImageCount > 0 && surfaceCapabilities.minImageCount + 1 > surfaceCapabilities.maxImageCount) ?
            surfaceCapabilities.maxImageCount : surfaceCapabilities.minImageCount + 1;

        desiredExtent = surfaceCapabilities.currentExtent;
        {
            // Special value of context.presentSurface extent is width == height == -1
            // If this is so we define the size by ourselves but it must fit within defined confines
            if (surfaceCapabilities.currentExtent.width == (uint32_t)(-1))
            {
                VkExtent2D swap_chain_extent = { 640, 480 };
                if (swap_chain_extent.width < surfaceCapabilities.minImageExtent.width)
                    swap_chain_extent.width = surfaceCapabilities.minImageExtent.width;
                if (swap_chain_extent.height < surfaceCapabilities.minImageExtent.height)
                    swap_chain_extent.height = surfaceCapabilities.minImageExtent.height;
                if (swap_chain_extent.width > surfaceCapabilities.maxImageExtent.width)
                    swap_chain_extent.width = surfaceCapabilities.maxImageExtent.width;
                if (swap_chain_extent.height > surfaceCapabilities.maxImageExtent.height)
                    swap_chain_extent.height = surfaceCapabilities.maxImageExtent.height;
                desiredExtent = swap_chain_extent;
            }
        }

        desiredUsage =
            surfaceCapabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferDst ?
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst : (vk::ImageUsageFlagBits)(-1);

        desiredTransform =
            surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity ?
            vk::SurfaceTransformFlagBitsKHR::eIdentity : surfaceCapabilities.currentTransform;
    }

    // set presentation mode
    vk::PresentModeKHR desiredPresentMode = (vk::PresentModeKHR) - 1;
    {
        const std::vector<vk::PresentModeKHR>& presentModes =
            m_deviceManager.GetPhysicalDevice().getSurfacePresentModesKHR(m_deviceManager.GetPresentSurface(), m_dispatcher);

        // FIFO present mode is always available
        // MAILBOX is the lowest latency V-Sync enabled mode (something like triple-buffering) so use it if available
        for (vk::PresentModeKHR presentMode : presentModes)
        {
            if (presentMode == vk::PresentModeKHR::eFifo/*eMailbox*/)
            {
                desiredPresentMode = presentMode;
                break;
            }
        }
        if (desiredPresentMode != vk::PresentModeKHR::eMailbox)
        {
            for (vk::PresentModeKHR presentMode : presentModes)
            {
                if (presentMode == vk::PresentModeKHR::eFifo)
                {
                    desiredPresentMode = presentMode;
                    break;
                }
            }
        }
    }

    // check that we have valid values
    if (desiredUsage == (vk::ImageUsageFlagBits)(-1)) {
        return false;
    }
    if ((int)desiredPresentMode == -1) {
        return false;
    }
    if ((desiredExtent.width == 0) || (desiredExtent.height == 0)) {
        // Current context.presentSurface size is (0, 0) so we can't create a swap chain and render anything (CanRender == false)
        // But we don't wont to kill the application as this situation may occur i.e. when window gets minimized
        return true;
    }

    vk::SwapchainCreateInfoKHR swapChainCreateInfo(
        vk::SwapchainCreateFlagsKHR(),
        m_deviceManager.GetPresentSurface(),
        desiredImageCount,
        desiredFormat.format,
        desiredFormat.colorSpace,
        desiredExtent,
        1, // numLayers
        desiredUsage,
        vk::SharingMode::eExclusive,
        0, // queueFamilyIndexCount
        nullptr, // queueFamilyIndices
        desiredTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        desiredPresentMode,
        true, // clipped
        m_swapChainInfo.swapChainHandle.get());

    m_swapChainInfo.swapChainHandle = 
        m_deviceManager.GetDevice().createSwapchainKHRUnique(swapChainCreateInfo, nullptr, m_dispatcher);

    // set swap chain images
    {
        m_swapChainInfo.format = desiredFormat.format;
        m_swapChainInfo.extent = desiredExtent;
        const std::vector<vk::Image>& images = m_deviceManager.GetDevice().getSwapchainImagesKHR(
            m_swapChainInfo.swapChainHandle.get(), m_dispatcher);

        for (vk::Image image : images)
            m_swapChainInfo.imageRefs.emplace_back(image);

        for (VulkanUtils::SwapChainInfo::ImageRef& imageInfo : m_swapChainInfo.imageRefs)
        {
            vk::ImageViewCreateInfo viewCreateInfo(
                vk::ImageViewCreateFlags(),
                imageInfo.image,
                vk::ImageViewType::e2D,
                m_swapChainInfo.format,
                { vk::ComponentSwizzle::eIdentity,
                vk::ComponentSwizzle::eIdentity,
                vk::ComponentSwizzle::eIdentity,
                vk::ComponentSwizzle::eIdentity },
                { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

            imageInfo.view = 
                m_deviceManager.GetDevice().createImageViewUnique(viewCreateInfo, nullptr, m_dispatcher);
        }
    }

    if (!VulkanUtils::CreateDepthImage(m_deviceManager, 
            m_swapChainInfo.extent.width, m_swapChainInfo.extent.height, m_depthImageInfo))
        return false;

    m_canDraw = true;

    return true;
}

bool
SwapChainRenderer::UpdateSwapChain()
{
    HEPHAESTUS_LOG_ASSERT(m_deviceManager.GetDevice(), "No Vulkan device available");
    m_deviceManager.WaitDevice();

    if (!CreateSwapChain())
        return false;

    return true;
}

} // namespace hephaestus