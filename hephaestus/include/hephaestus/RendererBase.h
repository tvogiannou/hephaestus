# pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/VulkanUtils.h>

#include <array>


namespace hephaestus
{

// Base class with common data for rendering pipelines
class RendererBase
{
public:

    using Color4 = std::array<float, 4>;

protected:
    explicit RendererBase(const VulkanDeviceManager& _deviceManager) :
        m_deviceManager(_deviceManager),
        m_dispatcher(m_deviceManager.GetDispatcher())
    {}
    ~RendererBase() { Clear(); }

    struct InitInfo
    {
        vk::ImageLayout outputImageLayout;
        vk::Format colorFormat = vk::Format::eR8G8B8A8Unorm;
        Color4 colorClearValues = { { 0.7f, 0.88f, 0.9f, 1.0f } };
    };
    bool Init(const InitInfo& info);
    void Clear();

public:
    void SetClearColor(const Color4& colorValues) { m_colorClearValues = colorValues; }

    const Color4& GetClearColor() const { return m_colorClearValues; }
    const VulkanDeviceManager& GetDeviceManager() const { return m_deviceManager; }
    const vk::CommandBuffer GetCmdBuffer() const { return m_cmdBuffer.get(); }
    const vk::RenderPass GetRenderPass() const { return m_renderPass.get(); }

protected:
    const VulkanDeviceManager&          m_deviceManager;
    const VulkanDispatcher&     m_dispatcher;   // member var for convenient access

    Color4                              m_colorClearValues;
    VulkanUtils::CommandPoolHandle      m_graphicsCommandPool;
    VulkanUtils::CommandBufferHandle    m_cmdBuffer;
    VulkanUtils::RenderPassHandle       m_renderPass;
    VulkanUtils::ImageInfo              m_depthImageInfo;

private:
    // non-copyable
    RendererBase(const RendererBase&) = delete;
    void operator=(const RendererBase&) = delete;
};

}