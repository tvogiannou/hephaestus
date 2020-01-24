#pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/VulkanFunctionDispatcher.h>
#include <hephaestus/VulkanGraphicsPipelineBase.h>


namespace hephaestus
{

// Graphics pipeline that renders primitive types (points & lines)
class VulkanPrimitiveGraphicsPipeline : public VulkanGraphicsPipelineBase
{
public:

    struct VertexData
    {
        static const uint32_t IndexSize = sizeof(uint32_t);

        float   x, y, z;		// position
        float   r, g, b/*, a*/;	// vertex color
    };

public:
    explicit VulkanPrimitiveGraphicsPipeline(const VulkanDeviceManager& _deviceManager) :
		VulkanGraphicsPipelineBase(_deviceManager)
	{}

	~VulkanPrimitiveGraphicsPipeline() { Clear(); }
	void Clear();

    // render data setup
	bool SetupPipeline(vk::RenderPass renderPass, const VulkanGraphicsPipelineBase::ShaderParams& shaderParams);

    // set line data
    bool AddLineStripData(const VulkanUtils::BufferUpdateInfo& updateInfo);

	// base class override
	void RecordDrawCommands(const VulkanUtils::FrameUpdateInfo& frameInfo) const override;

private:

    bool CreatePipeline(vk::RenderPass renderPass, const VulkanGraphicsPipelineBase::ShaderParams& shaderParams);

	// rendering pipeline setup
    VulkanUtils::PipelineHandle m_vulkanGraphicsPipeline;
    VulkanUtils::PipelineLayoutHandle m_graphicsPipelineLayout;

    std::vector<VkDeviceSize> m_lineStripOffsets;  // offsets to the vertex buffer
};

}