#pragma once

#include <common/AxisAlignedBoundingBox.h>
#include <hephaestus/TriMeshPipeline.h>
#include <hephaestus/PrimitivesPipeline.h>
#include <hephaestus/VulkanUtils.h>

#include <vector>


namespace hephaestus
{

// Simple tools and types for dealing with mesh data
struct MeshUtils
{
    // Container for triangle mesh
	struct TriMesh
	{
		std::vector<float> vertexData;  // in all following utils, this buffer is assumed to contain
                                        // vertices in the VulkanMeshGraphicsPipeline::VertexData format
        std::vector<uint32_t> indices;  // "flattened" triangle indices

		uint32_t vertexCount = 0;       // cached number of vertices
	};

    // Container with the extra info describing an image (but not the data buffer)
    struct ImageDesc
    {
        uint32_t width;
        uint32_t height;
        uint32_t numComponents;     // number of color components, e.g. 3 for RGB
    };

    static bool ComputeSmoothNormals(TriMesh& mesh);
    //static bool ApplyTransform(const Matrix4& transform, TriMesh& mesh);
    static AxisAlignedBoundingBox ComputeBoundingBox(const TriMesh& trimesh);

    static bool SetupPipelineForMesh(
        const TriMesh& mesh,
        const std::vector<char>& imageData, const ImageDesc& imageDesc,
        vk::CommandBuffer copyCmdBuffer,
        vk::RenderPass renderPass,
        const PipelineBase::ShaderParams& shaderParams,
        const TriMeshPipeline::SetupParams& pipelineParams,
        TriMeshPipeline& outPipeline);

    static bool SetupPrimitivesPipeline(
        std::vector<PrimitivesPipeline::VertexData> vertexData,
        vk::CommandBuffer copyCmdBuffer,
        vk::RenderPass renderPass,
        const PipelineBase::ShaderParams& shaderParams,
        PrimitivesPipeline& outPipeline);
};

}