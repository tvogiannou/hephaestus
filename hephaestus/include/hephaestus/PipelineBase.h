#pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanPlatformConfig.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/VulkanUtils.h>

#include <array>


namespace hephaestus
{
// Base class acting as a helper for all graphics pipelines
// - stage buffer to re-use
// - vertex buffer
// - descriptor pool
// - uniform buffer object data for passing model view matrix to shader
class PipelineBase
{
public:

    struct /*alignas(16)*/ UBOData
    {
        static const uint32_t UniformSize = 36 * sizeof(float);

        std::array<float, 16> projection = { { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f } };
        std::array<float, 16> model = { { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f } };
        std::array<float, 4> lightPos = { { 0.0f, 1.0f, 5.0f, 1.0f } };
    };

    struct ShaderParams
    {
        explicit ShaderParams(const VulkanUtils::ShaderDB& _shaderDB) : shaderDB(_shaderDB) {}

        const VulkanUtils::ShaderDB& shaderDB;
        VulkanUtils::ShaderDB::ShaderDBIndex vertexShaderIndex;
        VulkanUtils::ShaderDB::ShaderDBIndex fragmentShaderIndex;
    };

    explicit PipelineBase(const VulkanDeviceManager& _deviceManager) :
        m_deviceManager(_deviceManager),
        m_vertexBufferCurSize(0u)
    {}
    virtual ~PipelineBase() { Clear(); }

    void Clear();

    // buffer setup & update
    void CreateDescriptorPool(uint32_t uniformSize = 5, uint32_t combinedImgSamplerSize = 5);
    void CreateStageBuffer(uint32_t stageSize = 1000000);
    bool CreateVertexBuffer(uint32_t size);
    bool CreateUniformBuffer(uint32_t bufferSize);
    bool UpdateUniformBufferData(const VulkanUtils::BufferUpdateInfo& updateInfo);
    bool AppendVertexData(const VulkanUtils::BufferUpdateInfo& updateInfo);

    // update UBO data
    bool UpdateProjectionMatrix(const std::array<float, 16>& projectionMatrix, vk::CommandBuffer copyCmdBuffer);
    bool UpdateViewMatrix(const std::array<float, 16>& viewMatrix, vk::CommandBuffer copyCmdBuffer);
    bool UpdateViewAndProjectionMatrix(
        const std::array<float, 16>& viewMatrix, const std::array<float, 16>& projectionMatrix, vk::CommandBuffer copyCmdBuffer);
    bool UpdateLightPos(const std::array<float, 4>& lightPos, vk::CommandBuffer copyCmdBuffer);
    bool UpdateUBO(vk::CommandBuffer copyCmdBuffer);

    // internal
    vk::DescriptorPool GetDescriptorPool() const { return m_descriptorPool.get(); }
    const VulkanUtils::BufferInfo& GetVertexBufferInfo() const { return m_vertexBufferInfo; }
    const VulkanDeviceManager& GetDeviceManager() const { return m_deviceManager; }

    // override to setup more descriptor sets
    // TODO: make this less error prone
    virtual bool SetupDescriptorSets();

protected:

    void GetShaderModules(const PipelineBase::ShaderParams& shaderParams,
        vk::ShaderModule& vertexShaderModule, vk::ShaderModule& fragmentShaderModule);

    const VulkanDeviceManager& m_deviceManager;

    // descriptor set data
    VulkanUtils::DescriptorPoolHandle m_descriptorPool;
    VulkanUtils::DescriptorSetLayoutHandle m_descriptorSetLayout;
    VulkanUtils::DescriptorSetInfo m_descriptorSetInfo;

    // uniform buffer object data with model view transform + a light/general purpose transform
    VulkanUtils::BufferInfo m_uniformBufferInfo;
    UBOData m_uboData;

    // data buffers
    VulkanUtils::BufferInfo m_stageBufferInfo;	// stage buffer for temporary data
    VulkanUtils::BufferInfo m_vertexBufferInfo;
    VkDeviceSize m_vertexBufferCurSize; // size of data (in byte) currently set in the vertex buffer


private: 
    // non-copyable
    PipelineBase(const PipelineBase&) = delete;
    void operator=(const PipelineBase&) = delete;
};

}