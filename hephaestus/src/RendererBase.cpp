#include <hephaestus/RendererBase.h>

#include <hephaestus/Log.h>
#include <hephaestus/VulkanDispatcher.h>


namespace hephaestus
{

bool 
RendererBase::Init(const InitInfo& info)
{
    HEPHAESTUS_LOG_ASSERT(m_deviceManager.GetDevice(), "No Vulkan device available");
    m_deviceManager.WaitDevice();

    m_colorClearValues = info.colorClearValues;

    // create command pool
    {
        vk::CommandPoolCreateInfo cmdPoolCreateInfo(
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer |
            vk::CommandPoolCreateFlagBits::eTransient,
            m_deviceManager.GetGraphicsQueueInfo().familyIndex);
        m_graphicsCommandPool = m_deviceManager.GetDevice().createCommandPoolUnique(
            cmdPoolCreateInfo, nullptr);
    }

    // create command buffer used for copy operations
    {
        vk::CommandBufferAllocateInfo cmdBufferAllocateInfo(
            m_graphicsCommandPool.get(), vk::CommandBufferLevel::ePrimary, 1);
        std::vector<vk::CommandBuffer> buffer = m_deviceManager.GetDevice().allocateCommandBuffers(
            cmdBufferAllocateInfo);
        vk::PoolFree<vk::Device, vk::CommandPool, VulkanDispatcher> deleter(
            m_deviceManager.GetDevice(), m_graphicsCommandPool.get());
        m_cmdBuffer = VulkanUtils::CommandBufferHandle(buffer.front(), deleter);
    }

    if (!VulkanUtils::CreateRenderPass(m_deviceManager,
        info.colorFormat, info.outputImageLayout, m_renderPass))
        return false;

    return true;
}

void 
RendererBase::Clear()
{
    HEPHAESTUS_LOG_ASSERT(m_deviceManager.GetDevice(), "No Vulkan device available");
    m_deviceManager.WaitDevice();

    m_depthImageInfo.Clear();
    m_renderPass.reset(nullptr);
    m_cmdBuffer.reset(nullptr);
    m_graphicsCommandPool.reset(nullptr);
}

}