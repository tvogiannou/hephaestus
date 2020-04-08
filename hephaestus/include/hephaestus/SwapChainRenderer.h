#pragma once

#include <hephaestus/Compiler.h>
#include <hephaestus/PipelineBase.h>
#include <hephaestus/RendererBase.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/VulkanUtils.h>

#include <vector>
#include <utility>
#include <chrono>


namespace hephaestus
{

// Renderer class for targeting a swap chain (i.e. Vulkan abstraction for window systems)
// see https://vulkan.lunarg.com/doc/view/1.1.92.1/windows/tutorial/html/05-init_swapchain.html
class SwapChainRenderer : public RendererBase
{
public:

    struct RenderStats
    {
        float waitTime;     // total time waiting for available frame
        float commandTime;  // total time to record draw commands
        float queueTime;    // total time submitting the command buffer to the graphics queue
        float presentTime;  // total time submitting the command buffer to the present queue
    };

    enum RenderStatus : uint32_t
    {
        eRENDER_STATUS_COMPLETE = 0,        // Render stage has completed successfully
        eRENDER_STATUS_RESIZE,              // Render stage was interrupted due 
        eRENDER_STATUS_FAIL_WAIT_DEVICE,    // Failure while waiting for the device to finish any previous work
        eRENDER_STATUS_FAIL_ACQUIRE_IMAGE,  // Failure acquiring the image of the next available frame
        eRENDER_STATUS_FAIL_RESIZE,         // Failure while setting up the swap chain after a resize request
        eRENDER_STATUS_FAIL_PRESENT,        // Failure submitting the command buffer to the present queue
    };

    // Container for data relating to the rendering 
    // see RenderBegin()/End() below
    struct RenderInfo 
    {
        VulkanUtils::FrameUpdateInfo    frameInfo;  // info for the currently rendered frame

    private:
        friend class SwapChainRenderer;
        uint32_t    virtualFrameIndex;  // index to the currently rendered virtual frame 
                                        // (internal to class, should not be modified)
        uint32_t    imageIndex;         // index to the image of the swap chain that is rendered currently
                                        // (internal to class, should not be modified)
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