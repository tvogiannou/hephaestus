#pragma once

#include <common/CommonUtils.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/PrimitivesPipeline.h>
#include <hephaestus/SwapChainRenderer.h>
#include <hephaestus/VulkanUtils.h>


namespace hephaestus
{
class Camera;
class DebugState;
class RenderMeshData;


class VulkanWindowRenderer : public hephaestus::SwapChainRenderer
{
    // wrapper pipeline to render imgui via the renderer
    class ImGuiPipeline : public PipelineBase
    {
    public:
        explicit ImGuiPipeline(const VulkanDeviceManager& _deviceManager) :
            PipelineBase(_deviceManager)
        {}

        bool SetupPipeline(vk::RenderPass renderPass, vk::CommandPool commandPool, vk::CommandBuffer cmdBuffer);

        void RecordDrawCommands(const VulkanUtils::FrameUpdateInfo& frameInfo) const override;
    };

public:

    explicit VulkanWindowRenderer(const VulkanDeviceManager& _deviceManager) :
        SwapChainRenderer(_deviceManager),
        m_graphicsPipeline(_deviceManager),
        m_primitivePipeline(_deviceManager),
        m_imguiPipeline(_deviceManager)
    {}
    ~VulkanWindowRenderer() { Clear(); }

    bool Init();
    void Clear();

    // Utils used by main window
    bool Draw(float dtMsecs, SwapChainRenderer::RenderStats& stats);
    bool OnWindowSizeChanged();
    void UpdateCamera(const Camera& camera);

    // pipelines
    TriMeshPipeline m_graphicsPipeline;
    PrimitivesPipeline m_primitivePipeline;
    ImGuiPipeline m_imguiPipeline;
};

} // namespace hephaestus