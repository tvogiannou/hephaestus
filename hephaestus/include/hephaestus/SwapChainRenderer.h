#pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/PipelineBase.h>
#include <hephaestus/RendererBase.h>
#include <hephaestus/VulkanUtils.h>

#include <vector>
#include <utility>
#include <chrono>


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

    struct RenderInfo 
    {
        VulkanUtils::FrameUpdateInfo    frameInfo;
        uint32_t                        virtualFrameIndex;
        uint32_t                        imageIndex;
    };


public:
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

    // Rendering methods, any record commands can be called in between the two given the frame info
    RenderStatus RenderBegin(RenderInfo& renderInfo, RenderStats& stats);
    RenderStatus RenderEnd(const RenderInfo& renderInfo, RenderStats& stats);


    // Utility method for rendering arbitrary number of different pipeline types
    // only requirement is that the passed types support a method with signature
    // RecordDrawCommands(const VulkanUtils::FrameUpdateInfo& /*frameInfo*/) const
    template<typename... Args>
    static RenderStatus RenderPipelines(SwapChainRenderer& renderer, RenderStats& stats, Args&&... pipelines)
    {
        RenderInfo renderInfo;
        RenderStatus status = renderer.RenderBegin(renderInfo, stats);

        if (status != eRENDER_STATUS_COMPLETE)
            return status;

        auto timer_commandStart = std::chrono::high_resolution_clock::now();
       
        // expand variadic pack and call RecordDrawCommands for each argument
        // https://en.cppreference.com/w/cpp/language/parameter_pack
        {
            PackExpansionContext helper(renderInfo.frameInfo);

            using expander = int[];
            (void)expander{0, ((void)helper.RecordDrawCommands(std::forward<Args>(pipelines)), 0) ... };
        }

        auto timer_commandEnd = std::chrono::high_resolution_clock::now();

        stats.commandTime = std::chrono::duration<float, std::milli>(timer_commandEnd - timer_commandStart).count();
        
        status = renderer.RenderEnd(renderInfo, stats);

        return status;
    }

private:
    // rendering setup
    bool CreateSwapChain();
    bool UpdateSwapChain();
    bool CreateRenderingResources(uint32_t numVirtualFrames);

    // virtual frame info
    VulkanUtils::SwapChainInfo m_swapChainInfo;
    std::vector<VulkanUtils::VirtualFrameResources> m_virtualFrames;
    uint32_t m_nextAvailableVirtualFrameIndex = 0;

private:
    bool m_canDraw = false; // volatile?

    // helper type for storing the frame info of each call to RecordDrawCommands
    struct PackExpansionContext
    {
        PackExpansionContext(const VulkanUtils::FrameUpdateInfo& _frameInfo) :
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