#include <hephaestus/VulkanMeshGraphicsPipeline.h>

#include <hephaestus/Log.h>

#include <vector>


namespace hephaestus
{

void 
VulkanMeshGraphicsPipeline::RecordDrawCommands(const VulkanUtils::FrameUpdateInfo& frameInfo) const
{
    // bind pipeline
    frameInfo.drawCmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_vulkanGraphicsPipeline.get(), m_dispatcher);

    // draw the indexed vertex buffer
    if (m_vertexBufferInfo.bufferHandle && m_indexBufferInfo.bufferHandle)
    {
        frameInfo.drawCmdBuffer.bindVertexBuffers(0, m_vertexBufferInfo.bufferHandle.get(), (VkDeviceSize)0u, m_dispatcher);
        frameInfo.drawCmdBuffer.bindIndexBuffer(m_indexBufferInfo.bufferHandle.get(), (VkDeviceSize)0u, vk::IndexType::eUint32, m_dispatcher);

        for (size_t i = 0; i < m_subMeshes.size(); ++i)
        {
            const SubMeshInfo& info = m_subMeshes[i];
            if (info.visible)
            {
                HEPHAESTUS_LOG_ASSERT(info.indexOffset >= 0, "Cannot bind sub mesh without valid index offset");
                HEPHAESTUS_LOG_ASSERT(info.descriptorSetInfo.handle, "Cannot bind sub mesh without valid descriptor set");

                frameInfo.drawCmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                    m_pipelineLayout.get(), 0, info.descriptorSetInfo.handle.get(), nullptr, m_dispatcher);

                // compute offsets & index size from bytes to vertex indices as expected by drawIndexed()
                const uint32_t subMeshIndexSize =
                    (i + 1 == m_subMeshes.size() ? 
                    (uint32_t)m_indexBufferCurSize : 
                    (uint32_t)m_subMeshes[i + 1].indexOffset) - (uint32_t)info.indexOffset;

                const uint32_t indicesCount = subMeshIndexSize / VertexData::IndexSize;
                const uint32_t indexOffset = (uint32_t)info.indexOffset / VertexData::IndexSize;
                const uint32_t vertexOffset = (int32_t)info.vertexOffset / sizeof(VertexData);

                frameInfo.drawCmdBuffer.drawIndexed(indicesCount, 1, indexOffset, vertexOffset, 0, m_dispatcher);
            }
        }
    }
}

bool 
VulkanMeshGraphicsPipeline::SetupPipeline(vk::RenderPass renderPass, 
    const VulkanGraphicsPipelineBase::ShaderParams& shaderParams,
    const VulkanMeshGraphicsPipeline::SetupParams& params)
{
    if (!SetupDescriptorSets())
        return false;

    if (!CreatePipeline(renderPass, shaderParams, params))
        return false;

    return true;
}

bool 
VulkanMeshGraphicsPipeline::SetupDescriptorSets()
{
    if (!m_descriptorPool)
        return false;

    // create layouts
    {
        std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.emplace_back(    // uniform buffer binding
                0,
                vk::DescriptorType::eUniformBuffer,
                1,
                vk::ShaderStageFlagBits::eVertex,
                nullptr);
        layoutBindings.emplace_back(    // texture binding
            1,		// binding
            vk::DescriptorType::eCombinedImageSampler,
            1,		// count
            vk::ShaderStageFlagBits::eFragment,
            nullptr);		// immutable samplers
        
        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo(
            vk::DescriptorSetLayoutCreateFlagBits(), (uint32_t)layoutBindings.size(), layoutBindings.data());
        
        m_descriptorSetLayout =
            m_deviceManager.GetDevice().createDescriptorSetLayoutUnique(layoutCreateInfo, nullptr, m_dispatcher);
    }

    for (SubMeshInfo& info : m_subMeshes)
        SetupDescriptorSet(info.descriptorSetInfo, m_uniformBufferInfo, info.textureInfo);

    return true;
}

void 
VulkanMeshGraphicsPipeline::SetupDescriptorSet(VulkanUtils::DescriptorSetInfo& descSetInfo,
    const VulkanUtils::BufferInfo& uniformBufferInfo, const VulkanUtils::ImageInfo& textureInfo)
{
    // allocate descriptor set
    {
        vk::DescriptorSetAllocateInfo allocInfo(
            m_descriptorPool.get(), 1, &m_descriptorSetLayout.get());
        std::vector<vk::DescriptorSet> descSet = m_deviceManager.GetDevice().allocateDescriptorSets(allocInfo, m_dispatcher);
        vk::PoolFree<vk::Device, vk::DescriptorPool, VulkanFunctionDispatcher> deleter(
            m_deviceManager.GetDevice(), m_descriptorPool.get(), m_dispatcher);
        descSetInfo.handle = VulkanUtils::DescriptorSetHandle(descSet.front(), deleter);
    }

    vk::DescriptorImageInfo imageInfo;
    vk::DescriptorBufferInfo bufferInfo(
        uniformBufferInfo.bufferHandle.get(),
        0,		// offset
        uniformBufferInfo.size);

    std::vector<vk::WriteDescriptorSet> descriptorWrites;
    descriptorWrites.emplace_back(
        descSetInfo.handle.get(),
        0,		// destination binding
        0,		// destination array element
        1,		// descriptor count
        vk::DescriptorType::eUniformBuffer,
        nullptr,
        &bufferInfo,	// buffer info
        nullptr);
    if (textureInfo.imageHandle)
    {
        imageInfo = vk::DescriptorImageInfo(
            textureInfo.sampler.get(),
            textureInfo.view.get(),
            vk::ImageLayout::eShaderReadOnlyOptimal);

        descriptorWrites.emplace_back(
            descSetInfo.handle.get(),
            1,		// destination binding
            0,		// destination array element
            1,		// descriptor count
            vk::DescriptorType::eCombinedImageSampler,
            &imageInfo,
            nullptr,	// buffer info
            nullptr);
    }

    m_deviceManager.GetDevice().updateDescriptorSets(descriptorWrites, nullptr, m_dispatcher);
}

void
VulkanMeshGraphicsPipeline::CreatePipelineLayout()
{
    HEPHAESTUS_LOG_ASSERT(m_descriptorSetLayout, "Descriptor set layout is null");

    vk::PipelineLayoutCreateInfo layoutCreateInfo(
        vk::PipelineLayoutCreateFlags(),
        1, &m_descriptorSetLayout.get(),
        0, nullptr);
    m_pipelineLayout =
        m_deviceManager.GetDevice().createPipelineLayoutUnique(layoutCreateInfo, nullptr, m_dispatcher);
}

bool
VulkanMeshGraphicsPipeline::CreatePipeline(vk::RenderPass renderPass, 
    const VulkanGraphicsPipelineBase::ShaderParams& shaderParams,
    const VulkanMeshGraphicsPipeline::SetupParams& params)
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
    vk::VertexInputBindingDescription vertexBindingDescription(0, sizeof(VertexData), vk::VertexInputRate::eVertex);

    std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescription;
    vertexAttributeDescription.emplace_back(
            0,									// position
            vertexBindingDescription.binding,
            vk::Format::eR32G32B32Sfloat,
            (uint32_t)offsetof(struct VertexData, x));
    vertexAttributeDescription.emplace_back(
            1,									// normal
            vertexBindingDescription.binding,
            vk::Format::eR32G32B32Sfloat,
            (uint32_t)offsetof(struct VertexData, nx));
    vertexAttributeDescription.emplace_back(
            2,									// u,v coords
            vertexBindingDescription.binding,
            vk::Format::eR32G32Sfloat,
            (uint32_t)offsetof(struct VertexData, u));
    vertexAttributeDescription.emplace_back(
            3,									// color
            vertexBindingDescription.binding,
            vk::Format::eR32G32B32Sfloat,
            (uint32_t)offsetof(struct VertexData, r));

    vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo(
        vk::PipelineVertexInputStateCreateFlags(),
        1, &vertexBindingDescription,
        (uint32_t)vertexAttributeDescription.size(), vertexAttributeDescription.data());

    vk::PipelineInputAssemblyStateCreateInfo inputAssermblyCreateinfo(
        vk::PipelineInputAssemblyStateCreateFlags(),
        vk::PrimitiveTopology::eTriangleList,
        VK_FALSE);		// primitive restart		


    // rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo(
        vk::PipelineRasterizationStateCreateFlags(),
        VK_FALSE,							// depth clamp
        VK_FALSE,							// raster discard
        vk::PolygonMode::eFill,
        params.enableFaceCulling ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone,
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
    CreatePipelineLayout();

    // gather all pipeline params
    vk::GraphicsPipelineCreateInfo pipelineCreateInfo(
        vk::PipelineCreateFlags(),
        (uint32_t)shaderStageCreateInfos.size(),
        shaderStageCreateInfos.data(),
        &vertexInputStateCreateInfo,
        &inputAssermblyCreateinfo,
        nullptr,							// tessellation state
        &viewportStateCreateInfo,
        &rasterizationStateCreateInfo,
        &multisampleStateCreateInfo,
        &depthStencilCreateInfo,			// depth stencil state
        &colorBlendStateCreateInfo,
        &dynamicStateCreateInfo,			// dynamic state
        m_pipelineLayout.get(),
        renderPass,
        0,									// subpass
        nullptr,						    // basePipelineHandle
        -1);								// basePipelineIndex

    m_vulkanGraphicsPipeline = 
        m_deviceManager.GetDevice().createGraphicsPipelineUnique(nullptr, pipelineCreateInfo, nullptr, m_dispatcher);

    return true;
}

bool 
VulkanMeshGraphicsPipeline::SubMeshSetTextureData(SubMeshIDType subMeshId, 
    const VulkanUtils::TextureUpdateInfo& textureUpdateInfo)
{
    HEPHAESTUS_LOG_ASSERT(subMeshId < m_subMeshes.size(), "Sub mesh ID out of range");

    SubMeshInfo& subMeshInfo = m_subMeshes[subMeshId];
    return VulkanUtils::CopyImageDataStage(m_deviceManager, m_stageBufferInfo, subMeshInfo.textureInfo, textureUpdateInfo);
}

void 
VulkanMeshGraphicsPipeline::SubMeshSetVisible(uint32_t subMeshId, bool visible)
{
    HEPHAESTUS_LOG_ASSERT(subMeshId < m_subMeshes.size(), "Sub mesh ID out of range");

    SubMeshInfo& subMeshInfo = m_subMeshes[subMeshId];
    subMeshInfo.visible = visible;
}

void 
VulkanMeshGraphicsPipeline::Clear()
{
    HEPHAESTUS_LOG_ASSERT(m_deviceManager.GetDevice(), "No Vulkan device available");
    m_deviceManager.WaitDevice();

    m_indexBufferInfo.Clear();
    m_indexBufferCurSize = 0u;
    m_uniformBufferInfo.Clear();

    for (SubMeshInfo& info : m_subMeshes)
        info.Clear();
    m_subMeshes.clear();

    // make sure the descriptor pool is destroyed *after* we have destroyed the descriptor sets
    m_descriptorSetLayout.reset(nullptr);

    m_pipelineLayout.reset(nullptr);
    m_vulkanGraphicsPipeline.reset(nullptr);
    VulkanGraphicsPipelineBase::Clear();
}

bool 
VulkanMeshGraphicsPipeline::CreateIndexBuffer(uint32_t size)
{
    return VulkanUtils::CreateBuffer(m_deviceManager, size,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal, m_indexBufferInfo);
}

VulkanMeshGraphicsPipeline::SubMeshIDType 
VulkanMeshGraphicsPipeline::SubMeshCreate()
{
    m_subMeshes.push_back({});
    m_subMeshes.back().vertexOffset = m_vertexBufferCurSize;    // create always pointing at currently 
                                                                // available vertex buffer
    return (SubMeshIDType)(m_subMeshes.size() - 1);
}

bool 
VulkanMeshGraphicsPipeline::SubMeshCreateTexture(SubMeshIDType subMeshId, uint32_t width, uint32_t height)
{
    HEPHAESTUS_LOG_ASSERT(subMeshId < m_subMeshes.size(), "Sub mesh ID out of range");

    SubMeshInfo& subMeshInfo = m_subMeshes[subMeshId];
    return VulkanUtils::CreateImageTextureInfo(m_deviceManager, width, height, subMeshInfo.textureInfo);
}

bool
VulkanMeshGraphicsPipeline::SubMeshSetIndexData(SubMeshIDType subMeshId, 
    const VulkanUtils::BufferUpdateInfo& updateInfo)
{
    HEPHAESTUS_LOG_ASSERT(subMeshId < m_subMeshes.size(), "Sub mesh ID out of range");

    SubMeshInfo& subMeshInfo = m_subMeshes[subMeshId];
    HEPHAESTUS_LOG_ASSERT(subMeshInfo.indexOffset < 0, "Updating sub mesh index data but it has already been set");

    subMeshInfo.indexOffset = m_indexBufferCurSize;
    if (!VulkanUtils::CopyBufferDataStage(m_deviceManager, m_stageBufferInfo, updateInfo, m_indexBufferInfo, 
        vk::AccessFlagBits::eIndexRead, vk::PipelineStageFlagBits::eVertexInput, m_indexBufferCurSize))
        return false;

    m_indexBufferCurSize += updateInfo.dataSize;

    return true;
}

bool 
VulkanMeshGraphicsPipeline::SubMeshSetVertexData(SubMeshIDType subMeshId, 
    const VulkanUtils::BufferUpdateInfo& updateInfo)            
{
    HEPHAESTUS_LOG_ASSERT(subMeshId < m_subMeshes.size(), "Sub mesh ID out of range");

    SubMeshInfo& subMeshInfo = m_subMeshes[subMeshId];

    subMeshInfo.vertexOffset = m_vertexBufferCurSize;
    if (!VulkanUtils::CopyBufferDataHost(m_deviceManager, updateInfo, 
        m_vertexBufferInfo, m_vertexBufferCurSize))
        return false;

    m_vertexBufferCurSize += updateInfo.dataSize;

    return true;
}

bool 
VulkanMeshGraphicsPipeline::SubMeshUpdateVertexData(SubMeshIDType subMeshId, 
    const VulkanUtils::BufferUpdateInfo& updateInfo)
{
    HEPHAESTUS_LOG_ASSERT(subMeshId < m_subMeshes.size(), "Sub mesh ID out of range");

    SubMeshInfo& subMeshInfo = m_subMeshes[subMeshId];
    return VulkanUtils::CopyBufferDataHost(
        m_deviceManager, updateInfo, m_vertexBufferInfo, subMeshInfo.vertexOffset);
}

}