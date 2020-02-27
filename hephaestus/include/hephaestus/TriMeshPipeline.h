#pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/VulkanDispatcher.h>
#include <hephaestus/PipelineBase.h>

#include <vector>


namespace hephaestus
{
// Graphics pipeline that renders multiple (sub)meshes
// - position/normal/uv/color vertex buffer
// - projection & view matrix (using a uniform buffer)
class TriMeshPipeline : public PipelineBase
{
public:

    struct VertexData
    {
        static const uint32_t IndexSize = sizeof(uint32_t);

        float   x, y, z;    // position
        float	nx, ny, nz; // normal
        float	u, v;       // uv coords
        float   r, g, b;    // vertex color
    };

    struct SetupParams 
    {
        bool enableFaceCulling = true;
    };

public:
    TriMeshPipeline(const VulkanDeviceManager& _deviceManager) :
        PipelineBase(_deviceManager),
        m_indexBufferCurSize(0u)
    {}

    ~TriMeshPipeline() { Clear(); }
    void Clear();

    // Resources shared by all meshes
    bool CreateIndexBuffer(uint32_t size);

    // mesh API
    using MeshIDType = uint32_t;
    MeshIDType CreateMeshID();
    bool MeshCreateTexture(MeshIDType meshId, uint32_t width, uint32_t height);  // only support for RGBA
    bool MeshSetVertexData(MeshIDType meshId, const VulkanUtils::BufferUpdateInfo& updateInfo);
    bool MeshUpdateVertexData(MeshIDType meshId, const VulkanUtils::BufferUpdateInfo& updateInfo);
    bool MeshSetIndexData(MeshIDType meshId, const VulkanUtils::BufferUpdateInfo& updateInfo);
    bool MeshSetTextureData(MeshIDType meshId, const VulkanUtils::TextureUpdateInfo& textureUpdateInfo);
    void MeshSetVisible(uint32_t meshId, bool visible);

    // Setup internal/Vulkan data, call after setting all mesh data
    bool SetupPipeline(vk::RenderPass renderPass, 
        const PipelineBase::ShaderParams& shaderParams, const SetupParams& params);

    void RecordDrawCommands(const VulkanUtils::FrameUpdateInfo& frameInfo) const;

private:

    // pipeline setup
    bool SetupDescriptorSets() override;
    void SetupDescriptorSet(VulkanUtils::DescriptorSetInfo& descSetInfo, 
        const VulkanUtils::BufferInfo& uniformBufferInfo, const VulkanUtils::ImageInfo& textureInfo);

    bool CreatePipeline(vk::RenderPass renderPass, const PipelineBase::ShaderParams& shaderParams, const SetupParams& params);
    void CreatePipelineLayout();

    // rendering pipeline setup
    VulkanUtils::PipelineHandle m_vulkanGraphicsPipeline;
    VulkanUtils::PipelineLayoutHandle m_pipelineLayout;

    // meshes are sharing a vertex and an index buffer
    VulkanUtils::BufferInfo m_indexBufferInfo;
    VkDeviceSize m_indexBufferCurSize; // size of data (in byte) currently set in the index buffer
    struct MeshInfo
    {
        VkDeviceSize                    vertexOffset = 0u;  // offset in the vertex buffer
        int64_t                         indexOffset = -1;   // offset in the index buffer
        VulkanUtils::ImageInfo          textureInfo;        // info for the texture used for this mesh
        VulkanUtils::DescriptorSetInfo  descriptorSetInfo;  // descriptor set for this mesh
        bool                            visible = true;
        // TODO: dynamic model transform per mesh
    
        void Clear()
        {
            indexOffset = -1;
            textureInfo.Clear();
            descriptorSetInfo.Clear();
        }
    };
    std::vector<MeshInfo> m_MeshInfos;
};

}