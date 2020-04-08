#pragma once

#include <hephaestus/Compiler.h>
#include <hephaestus/RendererBase.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/VulkanUtils.h>


namespace hephaestus
{
class PipelineBase;

// Renderer class for targeting a windowless image buffer (headless)
class HeadlessRenderer : public RendererBase
{
public:

    HeadlessRenderer(const VulkanDeviceManager& deviceManager) :
        RendererBase(deviceManager)
    {}

    struct InitInfo : public RendererBase::InitInfo
    {
        uint32_t width = 1024u;
        uint32_t height = 1024u;
    };
    bool Init(const InitInfo& info);
    void Clear();
    const vk::Extent2D& GetExtent() const { return m_extent; }


    bool RenderBegin(VulkanUtils::FrameUpdateInfo& frameInfo) const;
    bool RenderEnd(const VulkanUtils::FrameUpdateInfo& frameInfo) const;

    // util to render single pipeline of any type
    // only requirement is that the passed types support a method with signature
    // RecordDrawCommands(const VulkanUtils::FrameUpdateInfo& /*frameInfo*/) const
    template<typename PipelineType>
    bool RenderPipeline(const PipelineType& pipeline) const
    {
        VulkanUtils::FrameUpdateInfo frameInfo;
        if (!RenderBegin(frameInfo))
            return false;

        pipeline.RecordDrawCommands(frameInfo);

        if (!RenderEnd(frameInfo))
            return false;

        CopyRenderFrame();

        return true;
    }

    void CopyRenderFrame() const;
    bool GetDstImageInfo(uint32_t& numChannels, uint32_t& width, uint32_t& height) const;
    bool GetDstImageData(char* data) const;

private:

    vk::Extent2D                    m_extent;           // size of the rendered target
    VulkanUtils::FramebufferHandle  m_framebuffer;      // buffer for the rendered frame during command buffer processing
    VulkanUtils::ImageInfo          m_frameImageInfo;   // image to use as render target
    VulkanUtils::ImageInfo          m_dstImageInfo;     // image that can be loaded from host memory
};

} // namespace hephaestus