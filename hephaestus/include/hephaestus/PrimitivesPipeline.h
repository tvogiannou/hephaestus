#pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/VulkanDispatcher.h>
#include <hephaestus/PipelineBase.h>


namespace hephaestus
{

// Graphics pipeline that renders primitive types (only lines at the moment)
class PrimitivesPipeline : public PipelineBase
{
public:
    struct VertexData
    {
        static const uint32_t IndexSize = sizeof(uint32_t);

        float   x, y, z;    // position
        float   r, g, b;    // vertex color
    };

public:
    explicit PrimitivesPipeline(const VulkanDeviceManager& _deviceManager) :
        PipelineBase(_deviceManager)
    {}

    ~PrimitivesPipeline() { Clear(); }
    void Clear();

    // render data setup
    bool SetupPipeline(vk::RenderPass renderPass, const PipelineBase::ShaderParams& shaderParams);

    // add a set of points that would be treated as part of a continuous line strip
    bool AddLineStripData(const VulkanUtils::BufferUpdateInfo& updateInfo);

    void RecordDrawCommands(const VulkanUtils::FrameUpdateInfo& frameInfo) const;

private:
    bool CreatePipeline(vk::RenderPass renderPass, const PipelineBase::ShaderParams& shaderParams);

    // rendering pipeline setup
    VulkanUtils::PipelineHandle m_vulkanGraphicsPipeline;
    VulkanUtils::PipelineLayoutHandle m_graphicsPipelineLayout;

    std::vector<VkDeviceSize> m_lineStripOffsets;  // offsets to the vertex buffer
};

}