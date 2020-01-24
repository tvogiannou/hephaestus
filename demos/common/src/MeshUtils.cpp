#include <common/MeshUtils.h>

#include <common/Vector3.h>
#include <hephaestus/VulkanMeshGraphicsPipeline.h>

namespace hephaestus
{

bool 
MeshUtils::ComputeSmoothNormals(TriMesh& mesh)
{
    using namespace hephaestus;

    const uint32_t step = sizeof(VulkanMeshGraphicsPipeline::VertexData) / sizeof(float);

    std::vector<Vector3> normals(mesh.vertexCount, Vector3::ZERO);
    for (size_t i = 0; i < mesh.indices.size(); i += 3u)
    {
        const uint32_t index0 = mesh.indices[i];
        const uint32_t index1 = mesh.indices[i + 1];
        const uint32_t index2 = mesh.indices[i + 2];

        const Vector3 p0(mesh.vertexData[index0 * step], 
                         mesh.vertexData[index0 * step + 1], 
                         mesh.vertexData[index0 * step + 2]);
        const Vector3 p1(mesh.vertexData[index1 * step],
                         mesh.vertexData[index1 * step + 1],
                         mesh.vertexData[index1 * step + 2]);
        const Vector3 p2(mesh.vertexData[index2 * step],
                         mesh.vertexData[index2 * step + 1],
                         mesh.vertexData[index2 * step + 2]);

        Vector3 e10 = p1;
        e10.Sub(p0);
        Vector3 e20 = p2;
        e20.Sub(p0);
        Vector3 normal = Vector3::Cross(e10, e20);

        normals[index0].Add(normal);
        normals[index1].Add(normal);
        normals[index2].Add(normal);
    }

    for (size_t i = 0u; i < normals.size(); ++i)
    {
        Vector3 n = normals[i];
        n.Normalize();

        mesh.vertexData[i * step + 3] = n.x;
        mesh.vertexData[i * step + 4] = n.y;
        mesh.vertexData[i * step + 5] = n.z;
    }

    return true;
}

AxisAlignedBoundingBox 
MeshUtils::ComputeBoundingBox(const TriMesh& trimesh)
{
    AxisAlignedBoundingBox bbox;
    bbox.Reset();

    const size_t step = sizeof(VulkanMeshGraphicsPipeline::VertexData) / sizeof(float);
    for (size_t i = 0u; i < trimesh.vertexData.size(); i += step)
    {
        bbox.AddPoint(Vector3(trimesh.vertexData[i],
                              trimesh.vertexData[i + 1],
                              trimesh.vertexData[i + 2]));
    }

    return bbox;
}

bool 
MeshUtils::SetupPipelineForMesh(const TriMesh& mesh, 
    const std::vector<char>& imageData, const ImageDesc& imageDesc,
    vk::CommandBuffer copyCmdBuffer, vk::RenderPass renderPass,
    const VulkanGraphicsPipelineBase::ShaderParams& shaderParams,
    const VulkanMeshGraphicsPipeline::SetupParams& pipelineParams,
    hephaestus::VulkanMeshGraphicsPipeline& outPipeline)
{
    outPipeline.Clear();
    outPipeline.CreateDescriptorPool();

    // allocate buffers for all data
    const uint32_t vertexDataSize =
        static_cast<uint32_t>(mesh.vertexData.size() * sizeof(float));
    const uint32_t indexDataSize =
        static_cast<uint32_t>(mesh.indices.size() * sizeof(uint32_t));

    constexpr uint32_t padding = 1000u;
    const uint32_t stageSize = std::max<uint32_t>(
        { vertexDataSize, indexDataSize }) + padding;
    outPipeline.CreateStageBuffer(stageSize);

    if (vertexDataSize > 0)
    {
        if (!outPipeline.CreateVertexBuffer(vertexDataSize))
            return false;
    }

    if (indexDataSize > 0)
    {
        if (!outPipeline.CreateIndexBuffer(indexDataSize))
            return false;
    }

    VulkanMeshGraphicsPipeline::SubMeshIDType subMeshId = outPipeline.SubMeshCreate();

    // update texture
    {
        outPipeline.SubMeshCreateTexture(subMeshId, imageDesc.width, imageDesc.height);

        VulkanUtils::TextureUpdateInfo updateInfo;
        {
            updateInfo.copyCmdBuffer = copyCmdBuffer;
            updateInfo.data = imageData.data();
            updateInfo.dataSize = (uint32_t)imageData.size();
            updateInfo.width = imageDesc.width;
            updateInfo.height = imageDesc.height;
        }
        outPipeline.SubMeshSetTextureData(subMeshId, updateInfo);
    }

    // upload vertex data
    {
        VulkanUtils::BufferUpdateInfo updateInfo;
        {
            updateInfo.copyCmdBuffer = copyCmdBuffer;
            updateInfo.data = reinterpret_cast<const char*>(mesh.vertexData.data());
            updateInfo.dataSize = vertexDataSize;
        }
        if (!outPipeline.SubMeshSetVertexData(subMeshId, updateInfo))
            return false;
    }

    // upload index data
    {
        VulkanUtils::BufferUpdateInfo updateInfo;
        {
            updateInfo.copyCmdBuffer = copyCmdBuffer;
            updateInfo.data = reinterpret_cast<const char*>(mesh.indices.data());
            updateInfo.dataSize = indexDataSize;
        }
        if (!outPipeline.SubMeshSetIndexData(subMeshId, updateInfo))
            return false;
    }

    // add uniform buffer
    {
        const uint32_t bufferSize = VulkanGraphicsPipelineBase::UBOData::UniformSize;
        if (!outPipeline.CreateUniformBuffer(bufferSize))
            return false;
    }

    // setup the pipeline
    if (!outPipeline.SetupPipeline(renderPass, shaderParams, pipelineParams))
        return false;

    return true;
}

bool 
MeshUtils::SetupPrimitivesPipeline(
    std::vector<VulkanPrimitiveGraphicsPipeline::VertexData> vertexData,
    vk::CommandBuffer copyCmdBuffer,
    vk::RenderPass renderPass,
    const VulkanGraphicsPipelineBase::ShaderParams& shaderParams, 
    VulkanPrimitiveGraphicsPipeline& outPipeline)
{
    using namespace hephaestus;

    outPipeline.Clear();
    outPipeline.CreateDescriptorPool();

    const uint32_t vertexDataSize =
        std::max<uint32_t>(4u, (uint32_t)vertexData.size()) * sizeof(VulkanPrimitiveGraphicsPipeline::VertexData);
    const uint32_t stageSize = 1024;    // stage not used atm

    outPipeline.CreateStageBuffer(stageSize);

    if (!outPipeline.CreateVertexBuffer(vertexDataSize))
        return false;

    const uint32_t bufferSize = VulkanGraphicsPipelineBase::UBOData::UniformSize;
    if (!outPipeline.CreateUniformBuffer(bufferSize))
        return false;

    VulkanUtils::BufferUpdateInfo updateInfo;
    {
        updateInfo.copyCmdBuffer = copyCmdBuffer;
        updateInfo.data = reinterpret_cast<const char*>(vertexData.data());
        updateInfo.dataSize =
            (uint32_t)vertexData.size() * sizeof(VulkanPrimitiveGraphicsPipeline::VertexData);
    }
    if (!outPipeline.AddLineStripData(updateInfo))
        return false;

    if (!outPipeline.SetupPipeline(renderPass, shaderParams))
        return false;

    return true;
}

}