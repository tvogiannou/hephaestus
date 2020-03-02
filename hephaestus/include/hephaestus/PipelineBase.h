#pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanConfig.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/VulkanUtils.h>

#include <array>


namespace hephaestus
{
// Base class acting as a helper for writing graphics pipelines
// - stage buffer to re-use
// - vertex buffer
// - descriptor pool
class PipelineBase
{
public:

    // helper for defining a small set of shaders and indices to the ones that should be used by the pipeline
    struct ShaderParams
    {
        explicit ShaderParams(const VulkanUtils::ShaderDB& _shaderDB) : shaderDB(_shaderDB) {}

        const VulkanUtils::ShaderDB& shaderDB;
        VulkanUtils::ShaderDB::ShaderDBIndex vertexShaderIndex;     // index to the shader module in shaderDB 
                                                                    // that should be used as the vertex shader
        VulkanUtils::ShaderDB::ShaderDBIndex fragmentShaderIndex;   // index to the shader module in shaderDB
                                                                    // that should be used as the fragment shader
    };

    explicit PipelineBase(const VulkanDeviceManager& _deviceManager) :
        m_deviceManager(_deviceManager),
        m_vertexBufferCurSize(0u)
    {}
    virtual ~PipelineBase() { Clear(); }

    void Clear();

    // buffer setup & update
    void CreateDescriptorPool(uint32_t uniformSize = 5u, uint32_t combinedImgSamplerSize = 5u);
    void CreateStageBuffer(uint32_t stageSize = 1000000u);
    bool CreateVertexBuffer(uint32_t size);
    bool AppendVertexData(const VulkanUtils::BufferUpdateInfo& updateInfo);

    // internal
    vk::DescriptorPool GetDescriptorPool() const { return m_descriptorPool.get(); }
    const VulkanUtils::BufferInfo& GetVertexBufferInfo() const { return m_vertexBufferInfo; }
    const VulkanDeviceManager& GetDeviceManager() const { return m_deviceManager; }

protected:

    void GetShaderModules(const PipelineBase::ShaderParams& shaderParams,
        vk::ShaderModule& vertexShaderModule, vk::ShaderModule& fragmentShaderModule);

    const VulkanDeviceManager& m_deviceManager;

    // descriptor set data
    VulkanUtils::DescriptorPoolHandle m_descriptorPool;
    VulkanUtils::DescriptorSetLayoutHandle m_descriptorSetLayout;
    VulkanUtils::DescriptorSetInfo m_descriptorSetInfo;

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