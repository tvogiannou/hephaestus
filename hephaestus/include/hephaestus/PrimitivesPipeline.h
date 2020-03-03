#pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/VulkanDispatcher.h>
#include <hephaestus/PipelineBase.h>

#include <array>


namespace hephaestus
{

// Graphics pipeline that renders primitive types (only lines at the moment)
class PrimitivesPipeline : public PipelineBase
{
public:
    // vertex data format for the primitives pipeline
    struct VertexData
    {
        static const uint32_t IndexSize = sizeof(uint32_t);

        float   x, y, z;    // position
        float   r, g, b;    // vertex color
    };

    // data format for the uniform buffer used by the primitives pipeline
    // [0-15]  -> 4x4 projection matrix
    // [16-31] -> 4x4 modelview matrix
    using PrimitiveUBData = VulkanUtils::UniformBufferData<32u * sizeof(float)>;

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

    // projection 4x4 matrix for primitives
    bool UpdateProjectionMatrix(const std::array<float, 16>& projectionMatrix, vk::CommandBuffer copyCmdBuffer);
    // global 4x4 view matrix for primitives, typically not needed
    bool UpdateViewMatrix(const std::array<float, 16>& viewMatrix, vk::CommandBuffer copyCmdBuffer);


private:
    bool CreateUniformBuffer();
    bool CreatePipeline(vk::RenderPass renderPass, const PipelineBase::ShaderParams& shaderParams);
    bool SetupDescriptorSets();

    // rendering pipeline setup
    VulkanUtils::PipelineHandle m_vulkanGraphicsPipeline;
    VulkanUtils::PipelineLayoutHandle m_graphicsPipelineLayout;
    
    // uniform buffer with view & projection matrix for all primitives rendered
    VulkanUtils::DescriptorSetLayoutHandle m_descriptorSetLayout;
    VulkanUtils::DescriptorSetInfo m_descriptorSetInfo;
    PrimitiveUBData m_uniformBufferData;

    std::vector<VkDeviceSize> m_lineStripOffsets;  // offsets to the vertex buffer
};

} // namespace hephaestus