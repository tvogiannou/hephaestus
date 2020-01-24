#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <common/CommonUtils.h>
#include <common/MeshUtils.h>
#include <hephaestus/VulkanMeshGraphicsPipeline.h>
#include <hephaestus/VulkanPlatformConfig.h>
#include <hephaestus/VulkanHeadlessRenderer.h>

#include <tuple>
#include <vector>


namespace hephaestus_bindings
{

struct Utils
{
    static bool CopyTriMeshToRenderMeshData(
            const float* vertexPositions, const size_t vertexCount,
            const uint32_t* indices, const size_t indicesCount,
            const float* vertexUV,   // optional, same count as vertices
            hephaestus::MeshUtils::TriMesh& mesh);

    static std::tuple<pybind11::array_t<uint8_t>, uint32_t, uint32_t, uint32_t>
        ExtractRendererDstImage(const hephaestus::VulkanHeadlessRenderer& renderer);
};

struct HEPHAESTUS_BINDINGS_Vec4
{
    float x, y, z, w;
};

class HEPHAESTUS_BINDINGS_MeshData
{
public:

    struct TextureInfo
    {
        std::vector<char> data;
        hephaestus::MeshUtils::ImageDesc desc;
    };

    HEPHAESTUS_BINDINGS_MeshData()
    {
        m_projectionMatrix.fill(0.f);
        m_modelviewMatrix.fill(0.f);
    }

    std::array<float, 16> m_projectionMatrix;
    std::array<float, 16> m_modelviewMatrix;

    hephaestus::MeshUtils::TriMesh m_trimesh;
    TextureInfo m_texture;
};

} // namespace hephaestus_bindings