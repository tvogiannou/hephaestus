#pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/PipelineBase.h>
#include <hephaestus/RendererBase.h>
#include <hephaestus/VulkanUtils.h>

#include <vector>


namespace hephaestus
{

class SwapChainRenderer : public RendererBase
{
public:

    using PipelineArray = std::vector<const PipelineBase*>;

    struct RenderStats
    {
        float waitTime;
        float commandTime;
        float queueTime;
        float presentTime;
    };

    enum RenderStatus : uint32_t
    {
        eRENDER_STATUS_COMPLETE = 0,
        eRENDER_STATUS_RESIZE,
        eRENDER_STATUS_FAIL_WAIT_DEVICE,
        eRENDER_STATUS_FAIL_ACQUIRE_IMAGE,
        eRENDER_STATUS_FAIL_RESIZE,
        eRENDER_STATUS_FAIL_PRESENT,
    };


    explicit SwapChainRenderer(const VulkanDeviceManager& _deviceManager) :
        RendererBase(_deviceManager)
    {}
    ~SwapChainRenderer() { Clear(); }

    struct InitInfo : public RendererBase::InitInfo
    {
        uint32_t numVirtualFrames = 3u;
    };
    bool Init(const InitInfo& info);
    void Clear();

    bool IsReadyToDraw() const { return m_canDraw; }
    const vk::Extent2D& GetSwapChainExtend() const { return m_swapChainInfo.extent; }

    RenderStatus RenderPipelines(const PipelineArray& pipelines, RenderStats& stats);

private:
    // rendering setup
    bool CreateSwapChain();
    bool UpdateSwapChain();
    bool CreateRenderingResources(uint32_t numVirtualFrames);
    void RecordPipelineCommands(const PipelineArray& pipelines, const VulkanUtils::FrameUpdateInfo& frameInfo);

    // virtual frame info
    VulkanUtils::SwapChainInfo m_swapChainInfo;
    std::vector<VulkanUtils::VirtualFrameResources> m_virtualFrames;
    uint32_t m_virtualFrameIndex = 0;

private:
    bool m_canDraw = false; // volatile?
};

} // namespace hephaestus