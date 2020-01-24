#pragma once

#include <common/CommonUtils.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/VulkanPrimitiveGraphicsPipeline.h>
#include <hephaestus/VulkanSwapChainRenderer.h>
#include <hephaestus/VulkanUtils.h>


namespace hephaestus
{
class Camera;
class DebugState;
class RenderMeshData;


class VulkanWindowRenderer : public hephaestus::VulkanSwapChainRenderer
{
    // wrapper pipeline to render imgui via the renderer
    class ImGuiPipeline : public VulkanGraphicsPipelineBase
    {
    public:
        explicit ImGuiPipeline(const VulkanDeviceManager& _deviceManager) :
            VulkanGraphicsPipelineBase(_deviceManager)
        {}

        bool SetupPipeline(vk::RenderPass renderPass, vk::CommandPool commandPool, vk::CommandBuffer cmdBuffer);

        void RecordDrawCommands(const VulkanUtils::FrameUpdateInfo& frameInfo) const override;
    };

public:

    explicit VulkanWindowRenderer(const VulkanDeviceManager& _deviceManager) :
        VulkanSwapChainRenderer(_deviceManager),
        m_graphicsPipeline(_deviceManager),
        m_primitivePipeline(_deviceManager),
        m_imguiPipeline(_deviceManager)
    {}
    ~VulkanWindowRenderer() { Clear(); }

    bool Init();
    void Clear();

    // Utils used by main window
    bool Draw(float dtMsecs, VulkanSwapChainRenderer::RenderStats& stats);
    bool OnWindowSizeChanged();
    void UpdateCamera(const Camera& camera);

    // pipelines
    VulkanMeshGraphicsPipeline m_graphicsPipeline;
    VulkanPrimitiveGraphicsPipeline m_primitivePipeline;
    ImGuiPipeline m_imguiPipeline;
};

} // namespace hephaestus