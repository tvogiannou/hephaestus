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
        vk::MemoryPropertyFlagBits::eHostVisible,   // host visible seems to work better (on both laptop with 
        m_vertexBufferInfo);                    // Intel UHD GPU & PC with RTX 2080) when rebuilding the mesh often
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

bool 
PipelineBase::SetupDescriptorSets()
{
    if (!m_descriptorPool)
        return false;

    // create layouts
    {
        std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.emplace_back(
            0,
            vk::DescriptorType::eUniformBuffer,
            1,
            vk::ShaderStageFlagBits::eVertex,
            nullptr);

        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo(
            vk::DescriptorSetLayoutCreateFlagBits(), (uint32_t)layoutBindings.size(), layoutBindings.data());

        m_descriptorSetLayout =
            m_deviceManager.GetDevice().createDescriptorSetLayoutUnique(layoutCreateInfo, nullptr);
    }

    // allocate descriptor set
    {
        vk::DescriptorSetAllocateInfo allocInfo(
            m_descriptorPool.get(), 1, &m_descriptorSetLayout.get());
        std::vector<vk::DescriptorSet> descSet =
            m_deviceManager.GetDevice().allocateDescriptorSets(allocInfo);
        vk::PoolFree<vk::Device, vk::DescriptorPool, VulkanDispatcher> deleter(
            m_deviceManager.GetDevice(), m_descriptorPool.get());
        m_descriptorSetInfo.handle =
            VulkanUtils::DescriptorSetHandle(descSet.front(), deleter);
    }

    vk::DescriptorImageInfo imageInfo;
    vk::DescriptorBufferInfo bufferInfo(
        m_uniformBufferInfo.bufferHandle.get(),
        0,		// offset
        m_uniformBufferInfo.size);

    std::vector<vk::WriteDescriptorSet> descriptorWrites;
    descriptorWrites.emplace_back(
        m_descriptorSetInfo.handle.get(),
        0,		// destination binding
        0,		// destination array element
        1,		// descriptor count
        vk::DescriptorType::eUniformBuffer,
        nullptr,
        &bufferInfo,	// buffer info
        nullptr);

    m_deviceManager.GetDevice().updateDescriptorSets(descriptorWrites, nullptr);

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

bool 
PipelineBase::CreateUniformBuffer(uint32_t bufferSize)
{
    return VulkanUtils::CreateBuffer(m_deviceManager, bufferSize,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible,
        m_uniformBufferInfo);
}

bool 
PipelineBase::UpdateUniformBufferData(const VulkanUtils::BufferUpdateInfo& updateInfo)
{
    return VulkanUtils::CopyBufferDataHost(m_deviceManager, updateInfo, m_uniformBufferInfo);
}

void 
PipelineBase::Clear()
{
    HEPHAESTUS_LOG_ASSERT(m_deviceManager.GetDevice(), "No Vulkan device available");
    m_deviceManager.WaitDevice();

    m_vertexBufferInfo.Clear();
    m_stageBufferInfo.Clear();


    m_uniformBufferInfo.Clear();
    m_descriptorSetInfo.Clear();
    m_descriptorSetLayout.reset(nullptr);  
    m_descriptorPool.reset(nullptr);     // make sure the descriptor pool is destroyed *after* we have destroyed the descriptor sets

    m_vertexBufferCurSize = 0u;
}

bool 
PipelineBase::UpdateProjectionMatrix(const std::array<float, 16>& projectionMatrix, vk::CommandBuffer copyCmdBuffer)
{
    m_uboData.projection = projectionMatrix;

    return UpdateUBO(copyCmdBuffer);
}

bool 
PipelineBase::UpdateViewMatrix(const std::array<float, 16>& viewMatrix, vk::CommandBuffer copyCmdBuffer)
{
    m_uboData.model = viewMatrix;

    return UpdateUBO(copyCmdBuffer);
}

bool
PipelineBase::UpdateViewAndProjectionMatrix(
    const std::array<float, 16>& viewMatrix, const std::array<float, 16>& projectionMatrix, vk::CommandBuffer copyCmdBuffer)
{
    m_uboData.model = viewMatrix;
    m_uboData.projection = projectionMatrix;

    return UpdateUBO(copyCmdBuffer);
}

bool
PipelineBase::UpdateLightPos(const std::array<float, 4>& lightPos, vk::CommandBuffer copyCmdBuffer)
{
    m_uboData.lightPos = lightPos;

    return UpdateUBO(copyCmdBuffer);
}

bool 
PipelineBase::UpdateUBO(vk::CommandBuffer copyCmdBuffer)
{
    const uint32_t buffSize = 
        VulkanUtils::FixupFlushRange(m_deviceManager, PipelineBase::UBOData::UniformSize);

    std::vector<char> tempBuffer;
    tempBuffer.resize(buffSize);
    {
        std::memcpy(&tempBuffer[0], m_uboData.projection.data(), 16 * sizeof(float));
        std::memcpy(&tempBuffer[16 * sizeof(float)], m_uboData.model.data(), 16 * sizeof(float));
        std::memcpy(&tempBuffer[32 * sizeof(float)], m_uboData.lightPos.data(), 4 * sizeof(float));
    }

    VulkanUtils::BufferUpdateInfo updateInfo;
    {
        updateInfo.copyCmdBuffer = copyCmdBuffer;
        updateInfo.data = tempBuffer.data();
        updateInfo.dataSize = buffSize;
    }

    return UpdateUniformBufferData(updateInfo);
}

}