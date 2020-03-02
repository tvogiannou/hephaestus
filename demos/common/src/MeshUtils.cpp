#include <common/MeshUtils.h>

#include <common/Vector3.h>
#include <hephaestus/TriMeshPipeline.h>

namespace hephaestus
{

bool 
MeshUtils::ComputeSmoothNormals(TriMesh& mesh)
{
    using namespace hephaestus;

    const uint32_t step = sizeof(TriMeshPipeline::VertexData) / sizeof(float);

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

    const size_t step = sizeof(TriMeshPipeline::VertexData) / sizeof(float);
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
    const PipelineBase::ShaderParams& shaderParams,
    const TriMeshPipeline::SetupParams& pipelineParams,
    hephaestus::TriMeshPipeline& outPipeline)
{
    outPipeline.Clear();
    outPipeline.CreateDescriptorPool();

    // allocate buffers for all data
    const uint32_t vertexDataSize = (uint32_t)(mesh.vertexData.size() * sizeof(float));
    const uint32_t vertexCopyDataSize = 
        VulkanUtils::FixupFlushRange(outPipeline.GetDeviceManager(), vertexDataSize);

    const uint32_t indexDataSize = (uint32_t)(mesh.indices.size() * sizeof(uint32_t));
    const uint32_t indexCopyDataSize = 
        VulkanUtils::FixupFlushRange(outPipeline.GetDeviceManager(), indexDataSize);
    
    // use temp buffer to data copy from so that vkFlushMappedMemoryRanges is always valid
    const uint32_t maxCopySize = std::max<uint32_t>({ vertexCopyDataSize, indexCopyDataSize });
    std::vector<char> tempBuffer(maxCopySize, 0);

    // allocate stage buffer
    constexpr uint32_t padding = 0x1000;
    const uint32_t stageSize = maxCopySize + padding;
    outPipeline.CreateStageBuffer(stageSize);

    if (vertexDataSize > 0)
    {
        if (!outPipeline.CreateVertexBuffer(vertexCopyDataSize))
            return false;
    }

    if (indexDataSize > 0)
    {
        if (!outPipeline.CreateIndexBuffer(indexCopyDataSize))
            return false;
    }

    TriMeshPipeline::MeshIDType newMeshId = outPipeline.CreateMeshID();

    // update texture
    {
        outPipeline.MeshCreateTexture(newMeshId, imageDesc.width, imageDesc.height);

        VulkanUtils::TextureUpdateInfo updateInfo;
        {
            updateInfo.copyCmdBuffer = copyCmdBuffer;
            updateInfo.data = imageData.data();
            updateInfo.dataSize = (uint32_t)imageData.size();
            updateInfo.width = imageDesc.width;
            updateInfo.height = imageDesc.height;
        }
        outPipeline.MeshSetTextureData(newMeshId, updateInfo);
    }

    // upload vertex data
    {
        VulkanUtils::BufferUpdateInfo updateInfo;
        {
            updateInfo.copyCmdBuffer = copyCmdBuffer;

            if (vertexDataSize < vertexCopyDataSize)
            {
                tempBuffer.assign(maxCopySize, 0u);
                std::memcpy(tempBuffer.data(), mesh.vertexData.data(), vertexDataSize);
                updateInfo.data = tempBuffer.data();
                updateInfo.dataSize = vertexCopyDataSize;
            }
            else
            {
                updateInfo.data = reinterpret_cast<const char*>(mesh.vertexData.data());
                updateInfo.dataSize = vertexDataSize;
            }
        }
        if (!outPipeline.MeshSetVertexData(newMeshId, updateInfo))
            return false;
    }

    // upload index data
    {
        VulkanUtils::BufferUpdateInfo updateInfo;
        {
            updateInfo.copyCmdBuffer = copyCmdBuffer;

            if (indexDataSize < indexCopyDataSize)
            {
                tempBuffer.assign(maxCopySize, 0u);
                std::memcpy(tempBuffer.data(), mesh.indices.data(), indexDataSize);
                updateInfo.data = tempBuffer.data();
                updateInfo.dataSize = indexDataSize;
            }
            else
            {
                updateInfo.data = reinterpret_cast<const char*>(mesh.indices.data());
                updateInfo.dataSize = indexDataSize;
            }
        }
        if (!outPipeline.MeshSetIndexData(newMeshId, updateInfo))
            return false;
    }

    // add uniform buffer
    {
        const uint32_t bufferSize = VulkanUtils::FixupFlushRange(
            outPipeline.GetDeviceManager(), TriMeshPipeline::UniformBufferData::UniformSize);
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
    std::vector<PrimitivesPipeline::VertexData> vertexData,
    vk::CommandBuffer copyCmdBuffer,
    vk::RenderPass renderPass,
    const PipelineBase::ShaderParams& shaderParams, 
    PrimitivesPipeline& outPipeline)
{
    using namespace hephaestus;

    outPipeline.Clear();
    outPipeline.CreateDescriptorPool();

    const uint32_t vertexDataSize =
        std::max<uint32_t>(4u, (uint32_t)vertexData.size()) * 
        sizeof(PrimitivesPipeline::VertexData);
    const uint32_t vertexCopyDataSize = 
        VulkanUtils::FixupFlushRange(outPipeline.GetDeviceManager(), vertexDataSize);
    std::vector<char> tempBuffer(vertexCopyDataSize, 0);

    const uint32_t stageSize = 0x1000;    // stage not used atm
    outPipeline.CreateStageBuffer(stageSize);

    if (!outPipeline.CreateVertexBuffer(vertexCopyDataSize))
        return false;

    const uint32_t bufferSize = VulkanUtils::FixupFlushRange(
            outPipeline.GetDeviceManager(), PrimitivesPipeline::UniformBufferData::UniformSize);
    if (!outPipeline.CreateUniformBuffer(bufferSize))
        return false;

    VulkanUtils::BufferUpdateInfo updateInfo;
    {
        updateInfo.copyCmdBuffer = copyCmdBuffer;

        if (vertexDataSize < vertexCopyDataSize)
        {
            std::memcpy(tempBuffer.data(), vertexData.data(), vertexDataSize);
            updateInfo.data = tempBuffer.data();
            updateInfo.dataSize = vertexCopyDataSize;
        }
        else
        {
            updateInfo.data = reinterpret_cast<const char*>(vertexData.data());
            updateInfo.dataSize =
                (uint32_t)vertexData.size() * sizeof(PrimitivesPipeline::VertexData);
        }
    }
    if (!outPipeline.AddLineStripData(updateInfo))
        return false;

    if (!outPipeline.SetupPipeline(renderPass, shaderParams))
        return false;

    return true;
}

}