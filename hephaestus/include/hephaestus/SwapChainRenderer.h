#pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/PipelineBase.h>
#include <hephaestus/RendererBase.h>
#include <hephaestus/VulkanUtils.h>

#include <vector>
#include <utility>


namespace hephaestus
{

class SwapChainRenderer : public RendererBase
{
public:

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

    struct ResourceUpdateInfo : public VulkanUtils::FrameUpdateInfo
    {
        uint32_t virtualFrameIndex;
        uint32_t imageIndex;
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

    RenderStatus RenderBegin(ResourceUpdateInfo& frameInfo, RenderStats& stats);
    RenderStatus RenderEnd(const ResourceUpdateInfo& frameInfo, RenderStats& stats);


    // Utility method for rendering arbitrary number of different pipeline types
    // only requirement is that the passed types support a method with signature
    // RecordDrawCommands(const VulkanUtils::FrameUpdateInfo& /*frameInfo*/) const
    template<typename... Args>
    static RenderStatus RenderPipelines(SwapChainRenderer& renderer, RenderStats& stats, Args&&... pipelines)
    {
        ResourceUpdateInfo updateInfo;
        renderer.RenderBegin(updateInfo, stats);

        //RenderPipelines(updateInfo, pipelines, stats);
        const VulkanUtils::FrameUpdateInfo& frameInfo = updateInfo;
        {
            // expand variadic pack 
            // https://en.cppreference.com/w/cpp/language/parameter_pack
            {
                PackExpansionHelper helper(frameInfo);

                using expander = int[];
                (void)expander{0, ((void)helper.RecordDrawCommands(std::forward<Args>(pipelines)), 0) ... };
            }
        }
        
        renderer.RenderEnd(updateInfo, stats);

        return RenderStatus::eRENDER_STATUS_COMPLETE;
    }

private:
    // rendering setup
    bool CreateSwapChain();
    bool UpdateSwapChain();
    bool CreateRenderingResources(uint32_t numVirtualFrames);

    // virtual frame info
    VulkanUtils::SwapChainInfo m_swapChainInfo;
    std::vector<VulkanUtils::VirtualFrameResources> m_virtualFrames;
    uint32_t m_virtualFrameIndex = 0;

private:
    bool m_canDraw = false; // volatile?

    // helper type for storing the frame info of each cal to RecordDrawCommands
    struct PackExpansionHelper
    {
        PackExpansionHelper(const VulkanUtils::FrameUpdateInfo& _frameInfo) :
            frameInfo(_frameInfo)
        {}

        template<typename PipelineType>
        void RecordDrawCommands(const PipelineType& pipeline)
        {
            pipeline.RecordDrawCommands(frameInfo);
        }

        const VulkanUtils::FrameUpdateInfo& frameInfo;
    };
};

} // namespace hephaestus