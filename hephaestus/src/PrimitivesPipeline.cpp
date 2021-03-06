#include <hephaestus/PrimitivesPipeline.h>

#include <hephaestus/Log.h>
#include <hephaestus/VulkanValidate.h>

#include <vector>


namespace hephaestus
{

void 
PrimitivesPipeline::RecordDrawCommands(const VulkanUtils::FrameUpdateInfo& frameInfo) const
{
    // bind pipeline
    frameInfo.drawCmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_vulkanGraphicsPipeline.get());

    // draw the vertex buffer
    if (m_vertexBufferInfo.IsValid() && m_vertexBufferCurSize > 0u)
    {
        frameInfo.drawCmdBuffer.bindVertexBuffers(0, m_vertexBufferInfo.bufferHandle.get(), (VkDeviceSize)0u);
        frameInfo.drawCmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
            m_graphicsPipelineLayout.get(), 0, m_descriptorSetInfo.handle.get(), nullptr);

        //frameInfo.drawCmdBuffer.setLineWidth(1.f);

        for (size_t i = 0u; i < m_lineStripOffsets.size(); ++i)
        {
            const VkDeviceSize start = i == 0 ? 0u : m_lineStripOffsets[i - 1];
            const VkDeviceSize count = m_lineStripOffsets[i] - start;

            const uint32_t startIndex = (uint32_t)(start / sizeof(VertexData));
            const uint32_t vertexCount = (uint32_t)(count / sizeof(VertexData));

            frameInfo.drawCmdBuffer.draw(vertexCount, 1, startIndex, 0u);
        }
    }
}

bool 
PrimitivesPipeline::UpdateProjectionMatrix(
    const std::array<float, 16>& projectionMatrix, vk::CommandBuffer copyCmdBuffer)
{
    std::memcpy(m_uniformBufferData.raw.data(), projectionMatrix.data(), 16u * sizeof(float));
    return VulkanUtils::UpdateUniformBufferData(m_deviceManager, m_uniformBufferData, copyCmdBuffer);
}

bool 
PrimitivesPipeline::UpdateViewMatrix(
    const std::array<float, 16>& viewMatrix, vk::CommandBuffer copyCmdBuffer)
{
    std::memcpy(&m_uniformBufferData.raw[16 * sizeof(float)], viewMatrix.data(), 16u * sizeof(float));
    return VulkanUtils::UpdateUniformBufferData(m_deviceManager, m_uniformBufferData, copyCmdBuffer);
}

bool 
PrimitivesPipeline::AddLineStripData(const VulkanUtils::BufferUpdateInfo& updateInfo)
{
    if (!VulkanUtils::CopyBufferDataHost(m_deviceManager, updateInfo, 
        m_vertexBufferInfo, m_vertexBufferCurSize))
        return false;

    m_vertexBufferCurSize += updateInfo.dataSize;
    m_lineStripOffsets.push_back(m_vertexBufferCurSize);

    return true;
}

bool 
PrimitivesPipeline::SetupPipeline(
    vk::RenderPass renderPass, const PipelineBase::ShaderParams& shaderParams)
{
    if (!CreateUniformBuffer())
        return false;

    if (!SetupDescriptorSets())
        return false;

    if (!CreatePipeline(renderPass, shaderParams))
        return false;

    return true;
}

bool
PrimitivesPipeline::CreatePipeline(
    vk::RenderPass renderPass, const PipelineBase::ShaderParams& shaderParams)
{
    HEPHAESTUS_LOG_ASSERT(renderPass, "No available render pass");

    // shaders
    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;
    GetShaderModules(shaderParams, vertexShaderModule, fragmentShaderModule);
    if (!vertexShaderModule || !fragmentShaderModule)
        return false;

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos;
    {
        shaderStageCreateInfos.emplace_back(
                vk::PipelineShaderStageCreateFlags(),
                vk::ShaderStageFlagBits::eVertex,
                vertexShaderModule,
                "main", nullptr);
        shaderStageCreateInfos.emplace_back(
                vk::PipelineShaderStageCreateFlags(),
                vk::ShaderStageFlagBits::eFragment,
                fragmentShaderModule,
                "main", nullptr);
    }


    // vertex & primitives input
    vk::VertexInputBindingDescription vertexBindingDescription(
        0, sizeof(PrimitivesPipeline::VertexData), vk::VertexInputRate::eVertex);

    std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescription;
    vertexAttributeDescription.emplace_back(
            0,									// position
            vertexBindingDescription.binding,
            vk::Format::eR32G32B32Sfloat,
            (uint32_t)offsetof(struct PrimitivesPipeline::VertexData, x));
    vertexAttributeDescription.emplace_back(
            1,									// color
            vertexBindingDescription.binding,
            vk::Format::eR32G32B32Sfloat,
            (uint32_t)offsetof(struct PrimitivesPipeline::VertexData, r));

    vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo(
        vk::PipelineVertexInputStateCreateFlags(),
        1, &vertexBindingDescription,
        (uint32_t)vertexAttributeDescription.size(), vertexAttributeDescription.data());

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateinfo(
        vk::PipelineInputAssemblyStateCreateFlags(),
        vk::PrimitiveTopology::eLineStrip,
        VK_FALSE);		// primitive restart		


    // rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo(
        vk::PipelineRasterizationStateCreateFlags(),
        VK_FALSE,							// depth clamp
        VK_FALSE,							// raster discard
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eNone,
        vk::FrontFace::eCounterClockwise,
        VK_FALSE, 0.f, 0.f, 0.f,			// depth bias
        1.f);								// line width


    // multi sampling
    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo(
        vk::PipelineMultisampleStateCreateFlags(),
        vk::SampleCountFlagBits::e1,		// raster samples
        VK_FALSE, 1.f, nullptr,				// sample shading
        VK_FALSE, VK_FALSE);				// alpha


    // blending
    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState(
        VK_FALSE,															// blendEnable
        vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,	// color blend
        vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,	// alpha blend
        vk::ColorComponentFlags(
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA));

    vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo(
        vk::PipelineColorBlendStateCreateFlags(),
        VK_FALSE, vk::LogicOp::eCopy,		// logicOp
        1, &colorBlendAttachmentState,
        { 0.f, 0.f, 0.f, 0.f });			// blendConstants


    // dynamic state for viewport & scissor (pass null but set size to 1)
    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo(
        vk::PipelineViewportStateCreateFlags(),
        1, nullptr,		// viewport
        1, nullptr);	// scissor

    std::vector<vk::DynamicState> dynamicStates = {
          vk::DynamicState::eViewport,
          vk::DynamicState::eScissor,
    };

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo(
        vk::PipelineDynamicStateCreateFlags(),
        (uint32_t)dynamicStates.size(), dynamicStates.data());

    vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo(
        vk::PipelineDepthStencilStateCreateFlags(),
        VK_TRUE,
        VK_TRUE,
        vk::CompareOp::eLess,
        VK_FALSE,
        VK_FALSE,
        {},
        {},
        0.f,
        1.f
    );

    // resources layout
    vk::PipelineLayoutCreateInfo layoutCreateInfo(
        vk::PipelineLayoutCreateFlags(),
        1, &m_descriptorSetLayout.get(),
        0, nullptr);
    HEPHAESTUS_CHECK_RESULT_HANDLE(m_graphicsPipelineLayout,
        m_deviceManager.GetDevice().createPipelineLayoutUnique(layoutCreateInfo, nullptr));

    // gather all pipeline params
    vk::GraphicsPipelineCreateInfo pipelineCreateInfo(
        vk::PipelineCreateFlags(),
        (uint32_t)shaderStageCreateInfos.size(),
        shaderStageCreateInfos.data(),
        &vertexInputStateCreateInfo,
        &inputAssemblyCreateinfo,
        nullptr,							// tessellation state
        &viewportStateCreateInfo,
        &rasterizationStateCreateInfo,
        &multisampleStateCreateInfo,
        &depthStencilCreateInfo,			// depth stencil state
        &colorBlendStateCreateInfo,
        &dynamicStateCreateInfo,			// dynamic state
        m_graphicsPipelineLayout.get(),
        renderPass,
        0,									// subpass
        nullptr,						    // basePipelineHandle
        -1);								// basePipelineIndex

    HEPHAESTUS_CHECK_RESULT_HANDLE(m_vulkanGraphicsPipeline, 
        m_deviceManager.GetDevice().createGraphicsPipelineUnique(nullptr, pipelineCreateInfo, nullptr));

    return true;
}

bool
PrimitivesPipeline::CreateUniformBuffer()
{
    const uint32_t bufferSize = VulkanUtils::FixupFlushRange(
        m_deviceManager, PrimitivesPipeline::PrimitiveUBData::UniformSize);

    return VulkanUtils::CreateBuffer(m_deviceManager, bufferSize,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible,
        m_uniformBufferData.bufferInfo);
}

bool
PrimitivesPipeline::SetupDescriptorSets()
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

        HEPHAESTUS_CHECK_RESULT_HANDLE(m_descriptorSetLayout,
            m_deviceManager.GetDevice().createDescriptorSetLayoutUnique(layoutCreateInfo, nullptr));
    }

    // allocate descriptor set
    {
        vk::DescriptorSetAllocateInfo allocInfo(
            m_descriptorPool.get(), 1, &m_descriptorSetLayout.get());
        std::vector<vk::DescriptorSet> descSet;
        HEPHAESTUS_CHECK_RESULT_RAW(descSet, m_deviceManager.GetDevice().allocateDescriptorSets(allocInfo));

        vk::PoolFree<vk::Device, vk::DescriptorPool, VulkanDispatcher> deleter(
            m_deviceManager.GetDevice(), m_descriptorPool.get());
        m_descriptorSetInfo.handle =
            VulkanUtils::DescriptorSetHandle(descSet.front(), deleter);
    }

    vk::DescriptorBufferInfo bufferInfo(
        m_uniformBufferData.bufferInfo.bufferHandle.get(),
        0,		// offset
        m_uniformBufferData.bufferInfo.size);

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
PrimitivesPipeline::Clear()
{
    HEPHAESTUS_LOG_ASSERT(m_deviceManager.GetDevice(), "No Vulkan device available");
    m_deviceManager.WaitDevice();

    m_lineStripOffsets.clear();

    // make sure the descriptor pool is destroyed *after* we have destroyed the descriptor sets
    m_uniformBufferData.bufferInfo.Clear();
    m_descriptorSetInfo.Clear();
    m_descriptorSetLayout.reset(nullptr);

    m_graphicsPipelineLayout.reset(nullptr);
    m_vulkanGraphicsPipeline.reset(nullptr);

    PipelineBase::Clear();
}

} // hephaestus