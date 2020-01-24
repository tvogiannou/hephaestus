#pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/VulkanRendererBase.h>
#include <hephaestus/VulkanUtils.h>


namespace hephaestus
{
class VulkanGraphicsPipelineBase;

class VulkanHeadlessRenderer : public VulkanRendererBase
{
public:

    VulkanHeadlessRenderer(const VulkanDeviceManager& deviceManager) :
        VulkanRendererBase(deviceManager)
    {}

    struct InitInfo : public VulkanRendererBase::InitInfo
    {
        uint32_t width = 1024u;
        uint32_t height = 1024u;
    };
    bool Init(const InitInfo& info);
    void Clear();

    bool RenderPipeline(const VulkanGraphicsPipelineBase& pipeline) const;

    const vk::Extent2D& GetExtent() const { return m_extent; }

    bool GetDstImageInfo(uint32_t& numChannels, uint32_t& width, uint32_t& height) const;
    bool GetDstImageData(char* data) const;

private:

    vk::Extent2D                    m_extent;
    VulkanUtils::ImageInfo          m_frameImageInfo;
    VulkanUtils::ImageInfo          m_dstImageInfo;
    VulkanUtils::FramebufferHandle  m_framebuffer;	// framebuffer to store target image during command buffer processing
};

} // namespace hephaestus