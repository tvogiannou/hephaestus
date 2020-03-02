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
    // vertex data format for the primitives pipeline
    struct VertexData
    {
        static const uint32_t IndexSize = sizeof(uint32_t);

        float   x, y, z;    // position
        float   r, g, b;    // vertex color
    };

    // data format for the uniform buffer used by the primitives pipeline
    struct UniformBufferData
    {
        static const uint32_t UniformSize = 32 * sizeof(float);

        std::array<float, 16> projection = { { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f } };
        std::array<float, 16> view = { { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f } };
    };

public:
    explicit PrimitivesPipeline(const VulkanDeviceManager& _deviceManager) :
        PipelineBase(_deviceManager)
    {}

    ~PrimitivesPipeline() { Clear(); }
    void Clear();

    // render data setup
    bool SetupPipeline(vk::RenderPass renderPass, const PipelineBase::ShaderParams& shaderParams);
    bool CreateUniformBuffer(uint32_t bufferSize);

    // add a set of points that would be treated as part of a continuous line strip
    bool AddLineStripData(const VulkanUtils::BufferUpdateInfo& updateInfo);

    void RecordDrawCommands(const VulkanUtils::FrameUpdateInfo& frameInfo) const;

    // projection 4x4 matrix for primitives
    bool UpdateProjectionMatrix(const std::array<float, 16>& projectionMatrix, vk::CommandBuffer copyCmdBuffer)
    {
        m_uniformbBufferData.projection = projectionMatrix;
        return PrimitivesPipeline::UpdateUniformBufferData(m_uniformBufferInfo, copyCmdBuffer);
    }
    // global 4x4 view matrix for primitives, typically not needed
    bool UpdateViewMatrix(const std::array<float, 16>& viewMatrix, vk::CommandBuffer copyCmdBuffer)
    {
        m_uniformbBufferData.view = viewMatrix;
        return PrimitivesPipeline::UpdateUniformBufferData(m_uniformBufferInfo, copyCmdBuffer);
    }


private:
    bool CreatePipeline(vk::RenderPass renderPass, const PipelineBase::ShaderParams& shaderParams);
    bool SetupDescriptorSets();
    bool UpdateUniformBufferData(const VulkanUtils::BufferInfo& uniformBufferInfo, vk::CommandBuffer copyCmdBuffer);

    // rendering pipeline setup
    VulkanUtils::PipelineHandle m_vulkanGraphicsPipeline;
    VulkanUtils::PipelineLayoutHandle m_graphicsPipelineLayout;
    
    // uniform buffer with modelview & projection matrix for all primitives rendered
    VulkanUtils::BufferInfo m_uniformBufferInfo;
    UniformBufferData m_uniformbBufferData;

    std::vector<VkDeviceSize> m_lineStripOffsets;  // offsets to the vertex buffer
};

} // namespace hephaestus