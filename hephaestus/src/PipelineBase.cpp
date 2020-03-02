#include <hephaestus/PipelineBase.h>

#include <hephaestus/Log.h>
#include <hephaestus/VulkanDispatcher.h>


namespace hephaestus
{

void
PipelineBase::CreateStageBuffer(uint32_t stageSize)
{
    VulkanUtils::CreateBuffer(m_deviceManager, stageSize,
        vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible, m_stageBufferInfo);
}

bool
PipelineBase::CreateVertexBuffer(uint32_t size)
{
    return VulkanUtils::CreateBuffer(m_deviceManager, size,
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible,   // eHostVisible is used to demonstrate updates of CPU mesh buffers 
        m_vertexBufferInfo);                        // for static meshes it should be more optimal to use eDeviceLocal
}

void 
PipelineBase::CreateDescriptorPool(uint32_t uniformSize /*= 5*/, uint32_t combinedImgSamplerSize /*= 5*/)
{
    VulkanUtils::CreateDescriptorPool(
        m_deviceManager, uniformSize, combinedImgSamplerSize, m_descriptorPool);
}

bool
PipelineBase::AppendVertexData(const VulkanUtils::BufferUpdateInfo& updateInfo)
{
    if (!VulkanUtils::CopyBufferDataHost(m_deviceManager, updateInfo, m_vertexBufferInfo))
        return false;

    m_vertexBufferCurSize += updateInfo.dataSize;

    return true;
}

void 
PipelineBase::GetShaderModules(
    const PipelineBase::ShaderParams& shaderParams, 
    vk::ShaderModule& vertexShaderModule, 
    vk::ShaderModule& fragmentShaderModule)
{
    vertexShaderModule = shaderParams.shaderDB.GetModule(shaderParams.vertexShaderIndex);
    fragmentShaderModule = shaderParams.shaderDB.GetModule(shaderParams.fragmentShaderIndex);
}

void 
PipelineBase::Clear()
{
    HEPHAESTUS_LOG_ASSERT(m_deviceManager.GetDevice(), "No Vulkan device available");
    m_deviceManager.WaitDevice();

    m_vertexBufferInfo.Clear();
    m_stageBufferInfo.Clear();


    m_descriptorSetInfo.Clear();
    m_descriptorSetLayout.reset(nullptr);  
    m_descriptorPool.reset(nullptr);     // make sure the descriptor pool is destroyed *after* we have destroyed the descriptor sets

    m_vertexBufferCurSize = 0u;
}

}