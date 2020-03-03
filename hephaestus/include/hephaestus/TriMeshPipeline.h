#pragma once

#include <hephaestus/Platform.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/VulkanDispatcher.h>
#include <hephaestus/PipelineBase.h>

#include <array>
#include <vector>


namespace hephaestus
{
// Graphics pipeline that renders multiple (sub)meshes
// - position/normal/uv/color vertex buffer
// - projection & view matrix (using a uniform buffer)
class TriMeshPipeline : public PipelineBase
{
public:
    using Matrix4x4f = std::array<float, 16>;
    using Vector4f = std::array<float, 4>;
    using MeshIDType = uint32_t;

    struct VertexData
    {
        static const uint32_t IndexSize = sizeof(uint32_t);

        float   x, y, z;    // position
        float	nx, ny, nz; // normal
        float	u, v;       // uv coords
        float   r, g, b;    // vertex color
    };

    // uniform buffer data for the entire scene (projection, view transforms & light)
    struct SceneUBData
    {
        static const uint32_t UniformSize = 48u * sizeof(float);

        // [0-15]  -> 4x4 projection matrix
        // [16-31] -> 4x4 camera/view matrix
        // [32-47] -> 16 float misc shader data (e.g. light source position)
        std::array<char, UniformSize> raw;

        VulkanUtils::BufferInfo         uniformBufferInfo;
    };

    // uniform buffer data per mesh (model transform)
    struct MeshUBData
    {
        static const uint32_t UniformSize = 16u * sizeof(float);

        // [0-15]  -> 4x4 projection matrix
        std::array<char, UniformSize>   raw;

        VulkanUtils::BufferInfo         uniformBufferInfo;
    };

    struct SetupParams 
    {
        bool enableFaceCulling = true;
    };

public:
    explicit TriMeshPipeline(const VulkanDeviceManager& _deviceManager) :
        PipelineBase(_deviceManager),
        m_indexBufferCurSize(0u)
    {}

    ~TriMeshPipeline() { Clear(); }
    void Clear();

    // Resources shared by all meshes
    bool CreateIndexBuffer(uint32_t size);

    // mesh API
    MeshIDType CreateMeshID();
    bool MeshCreateTexture(MeshIDType meshId, uint32_t width, uint32_t height);  // only support for RGBA
    bool MeshSetVertexData(MeshIDType meshId, const VulkanUtils::BufferUpdateInfo& updateInfo);
    bool MeshUpdateVertexData(MeshIDType meshId, const VulkanUtils::BufferUpdateInfo& updateInfo);
    bool MeshSetIndexData(MeshIDType meshId, const VulkanUtils::BufferUpdateInfo& updateInfo);
    bool MeshSetTextureData(MeshIDType meshId, const VulkanUtils::TextureUpdateInfo& textureUpdateInfo);
    void MeshSetVisible(uint32_t meshId, bool visible);

    // transform update API
    bool UpdateProjectionMatrix(const Matrix4x4f& projectionMatrix, vk::CommandBuffer copyCmdBuffer);
    bool UpdateViewMatrix(const Matrix4x4f& viewMatrix, vk::CommandBuffer copyCmdBuffer);
    bool UpdateLightPos(const Vector4f& lightPos, vk::CommandBuffer copyCmdBuffer);
    bool UpdateViewAndProjectionMatrix(const Matrix4x4f& viewMatrix, const Matrix4x4f& projectionMatrix, vk::CommandBuffer copyCmdBuffer);
    bool UpdateModelMatrix(MeshIDType meshId, const Matrix4x4f& modelMatrix, vk::CommandBuffer copyCmdBuffer);

    // Setup internal/Vulkan data, call after setting all mesh data
    bool SetupPipeline(vk::RenderPass renderPass, 
        const PipelineBase::ShaderParams& shaderParams, const SetupParams& params);

    void RecordDrawCommands(const VulkanUtils::FrameUpdateInfo& frameInfo) const;

private:
    // pipeline setup
    bool CreateUniformBuffer(VulkanUtils::BufferInfo& bufferInfo, uint32_t reqSize);
    bool UpdateUniformBufferData(const SceneUBData& bufferData, vk::CommandBuffer copyCmdBuffer);
    bool UpdateUniformBufferData(const MeshUBData& bufferData, vk::CommandBuffer copyCmdBuffer);
    bool SetupDescriptorSets();
    void SetupMeshDescriptorSet(VulkanUtils::DescriptorSetInfo& descSetInfo,
        const VulkanUtils::BufferInfo& uniformBufferInfo, const VulkanUtils::ImageInfo& textureInfo);
    void CreatePipelineLayout();
    bool CreatePipeline(vk::RenderPass renderPass, const PipelineBase::ShaderParams& shaderParams, const SetupParams& params);

    // rendering pipeline setup
    VulkanUtils::PipelineHandle         m_vulkanPipeline;
    VulkanUtils::PipelineLayoutHandle   m_pipelineLayout;
    SceneUBData                         m_sceneUBData; // uniform data for the entire scene (all meshes)

    // meshes are sharing a vertex and an index buffer
    VulkanUtils::BufferInfo                 m_indexBufferInfo;
    VkDeviceSize                            m_indexBufferCurSize; // size of data (bytes) currently set in the index buffer
    VulkanUtils::DescriptorSetLayoutHandle  m_meshDescSetLayout; // descriptor layout for mesh descriptor sets
    struct MeshInfo
    {
        bool                            visible = true;
        VkDeviceSize                    vertexOffset = 0u;  // offset in the vertex buffer
        int64_t                         indexOffset = -1;   // offset in the index buffer
        VulkanUtils::ImageInfo          textureInfo;        // info for the texture used for this mesh
        MeshUBData                      ubData;             // uniform buffer data for this mesh (model transform)
        VulkanUtils::DescriptorSetInfo  descriptorSetInfo;  // descriptor set for this mesh

        void Clear()
        {
            ubData.uniformBufferInfo.Clear();
            indexOffset = -1;
            textureInfo.Clear();
            descriptorSetInfo.Clear();
        }
    };
    std::vector<MeshInfo> m_meshInfos;
};

} // hephaestus