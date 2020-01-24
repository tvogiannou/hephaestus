#include <hephaestus/VulkanGraphicsPipelineBase.h>

#include <hephaestus/Log.h>


namespace hephaestus
{

void
VulkanGraphicsPipelineBase::CreateStageBuffer(uint32_t stageSize)
{
	VulkanUtils::CreateBuffer(m_deviceManager, stageSize,
		vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible, m_stageBufferInfo);
}

bool
VulkanGraphicsPipelineBase::CreateVertexBuffer(uint32_t size)
{
	return VulkanUtils::CreateBuffer(m_deviceManager, size,
		vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible,   // host visible seems to work better (on both laptop with 
        m_vertexBufferInfo);                    // Intel UHD GPU & PC with RTX 2080) when rebuilding the mesh often
}

void 
VulkanGraphicsPipelineBase::CreateDescriptorPool(uint32_t uniformSize /*= 5*/, uint32_t combinedImgSamplerSize /*= 5*/)
{
    VulkanUtils::CreateDescriptorPool(
        m_deviceManager, uniformSize, combinedImgSamplerSize, m_descriptorPool);
}

bool
VulkanGraphicsPipelineBase::AppendVertexData(const VulkanUtils::BufferUpdateInfo& updateInfo)
{
	if (!VulkanUtils::CopyBufferDataHost(m_deviceManager, updateInfo, m_vertexBufferInfo))
        return false;

    m_vertexBufferCurSize += updateInfo.dataSize;

    return true;
}

bool 
VulkanGraphicsPipelineBase::SetupDescriptorSets()
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
            m_deviceManager.GetDevice().createDescriptorSetLayoutUnique(layoutCreateInfo, nullptr, m_dispatcher);
    }

    // allocate descriptor set
    {
        vk::DescriptorSetAllocateInfo allocInfo(
            m_descriptorPool.get(), 1, &m_descriptorSetLayout.get());
        std::vector<vk::DescriptorSet> descSet =
            m_deviceManager.GetDevice().allocateDescriptorSets(allocInfo, m_dispatcher);
        vk::PoolFree<vk::Device, vk::DescriptorPool, VulkanFunctionDispatcher> deleter(
            m_deviceManager.GetDevice(), m_descriptorPool.get(), m_dispatcher);
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

    m_deviceManager.GetDevice().updateDescriptorSets(descriptorWrites, nullptr, m_dispatcher);

    return true;
}

void 
VulkanGraphicsPipelineBase::GetShaderModules(
    const VulkanGraphicsPipelineBase::ShaderParams& shaderParams, 
    vk::ShaderModule& vertexShaderModule, 
    vk::ShaderModule& fragmentShaderModule)
{
    vertexShaderModule = shaderParams.shaderDB.GetModule(shaderParams.vertexShaderIndex);
    fragmentShaderModule = shaderParams.shaderDB.GetModule(shaderParams.fragmentShaderIndex);
}

bool 
VulkanGraphicsPipelineBase::CreateUniformBuffer(uint32_t bufferSize)
{
    return VulkanUtils::CreateBuffer(m_deviceManager, bufferSize,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible,
        m_uniformBufferInfo);
}

bool 
VulkanGraphicsPipelineBase::UpdateUniformBufferData(const VulkanUtils::BufferUpdateInfo& updateInfo)
{
    return VulkanUtils::CopyBufferDataHost(m_deviceManager, updateInfo, m_uniformBufferInfo);
}

void 
VulkanGraphicsPipelineBase::Clear()
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
VulkanGraphicsPipelineBase::UpdateProjectionMatrix(const std::array<float, 16>& projectionMatrix, vk::CommandBuffer copyCmdBuffer)
{
    m_uboData.projection = projectionMatrix;

    return UpdateUBO(copyCmdBuffer);
}

bool 
VulkanGraphicsPipelineBase::UpdateViewMatrix(const std::array<float, 16>& viewMatrix, vk::CommandBuffer copyCmdBuffer)
{
    m_uboData.model = viewMatrix;

    return UpdateUBO(copyCmdBuffer);
}

bool
VulkanGraphicsPipelineBase::UpdateViewAndProjectionMatrix(
    const std::array<float, 16>& viewMatrix, const std::array<float, 16>& projectionMatrix, vk::CommandBuffer copyCmdBuffer)
{
    m_uboData.model = viewMatrix;
    m_uboData.projection = projectionMatrix;

    return UpdateUBO(copyCmdBuffer);
}

bool
VulkanGraphicsPipelineBase::UpdateLightPos(const std::array<float, 4>& lightPos, vk::CommandBuffer copyCmdBuffer)
{
    m_uboData.lightPos = lightPos;

    return UpdateUBO(copyCmdBuffer);
}

bool 
VulkanGraphicsPipelineBase::UpdateUBO(vk::CommandBuffer copyCmdBuffer)
{
    alignas(16) std::array<char, 256> tembBuffer;
    {
        std::memcpy(&tembBuffer[0], m_uboData.projection.data(), 16 * sizeof(float));
        std::memcpy(&tembBuffer[16 * sizeof(float)], m_uboData.model.data(), 16 * sizeof(float));
        std::memcpy(&tembBuffer[32 * sizeof(float)], m_uboData.lightPos.data(), 4 * sizeof(float));
    }

    VulkanUtils::BufferUpdateInfo updateInfo;
    {
        updateInfo.copyCmdBuffer = copyCmdBuffer;
        updateInfo.data = tembBuffer.data();
        updateInfo.dataSize = UBOData::UniformSize;
    }

    return UpdateUniformBufferData(updateInfo);
}

}