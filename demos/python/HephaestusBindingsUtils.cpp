#include "module.h"

#include <common/AxisAlignedBoundingBox.h>


// helper macro to check return values & throw an exception if fail
#define CHECK_EXIT_MSG(expr, msg)	\
if (!(expr)) { throw std::runtime_error(msg); }

namespace hephaestus_bindings
{

bool 
Utils::CopyTriMeshToRenderMeshData(
    const float* vertexPositions, const size_t vertexCount,
    const uint32_t* indices, const size_t indicesCount,
    const float* vertexUV,   // optional, same count as vertices
    hephaestus::MeshUtils::TriMesh& mesh)
{
    using namespace hephaestus;

    mesh.vertexData.clear();
    mesh.indices.clear();
    mesh.vertexData.reserve(vertexCount);
    mesh.indices.reserve(indicesCount);

    // copy indices
    for (uint32_t triIndex = 0; triIndex < indicesCount; triIndex += 3)
    {
        // 3 indices per tri
        const uint32_t triIndex0 = indices[triIndex];
        const uint32_t triIndex1 = indices[triIndex + 1u];
        const uint32_t triIndex2 = indices[triIndex + 2u];

        mesh.indices.push_back((uint32_t)triIndex0);
        mesh.indices.push_back((uint32_t)triIndex1);
        mesh.indices.push_back((uint32_t)triIndex2);
    }

    // copy vertices
    if (vertexUV != nullptr)
    {
        uint32_t uvIndex = 0u;
        for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; vertexIndex += 3)
        {
            // get the point xyz
            const float p0x = vertexPositions[vertexIndex];
            const float p0y = vertexPositions[vertexIndex + 1u];
            const float p0z = vertexPositions[vertexIndex + 2u];

            const float uvx = vertexUV[uvIndex];
            const float uvy = vertexUV[uvIndex + 1u];
            uvIndex += 2u;

            mesh.vertexData.insert(mesh.vertexData.end(), {
                p0x, p0y, p0z,  // pos 
                0.f, 0.f, 0.f,  // normal
                uvx, uvy,       // uvs
                1.f, 1.f, 1.f,  // color
                });
        }
    }
    else
    {
        for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; vertexIndex += 3)
        {
            // get the point xyz
            const float p0x = vertexPositions[vertexIndex];
            const float p0y = vertexPositions[vertexIndex + 1u];
            const float p0z = vertexPositions[vertexIndex + 2u];

            mesh.vertexData.insert(mesh.vertexData.end(), {
                p0x, p0y, p0z,  // pos 
                0.f, 0.f, 0.f,  // normal
                0.f, 0.f,       // uvs
                1.f, 1.f, 1.f,  // color
                });
        }
    }

    mesh.vertexCount = vertexCount / 3u;

    return true;
}

std::tuple<pybind11::array_t<uint8_t>, uint32_t, uint32_t, uint32_t>
Utils::ExtractRendererDstImage(const hephaestus::VulkanHeadlessRenderer& renderer)
{
    // return the data
    uint32_t numChannels = 0u;
    uint32_t width = 0u;
    uint32_t height = 0u;
    renderer.GetDstImageInfo(numChannels, width, height);

    const size_t size = numChannels * width * height;
    auto imageData = pybind11::array_t<uint8_t>(size);
    pybind11::buffer_info imageDataBufferInfo = imageData.request();

    // pybind11::print("ExtractRendererDstImage: Loading image with ", numChannels, " channels,", width, " width and ", height, " height");

    CHECK_EXIT_MSG(renderer.GetDstImageData(reinterpret_cast<char*>(imageDataBufferInfo.ptr)),
        "Failed to read rendered image data");

    return std::make_tuple(imageData, numChannels, width, height);
}

} // namespace hephaestus_bindings