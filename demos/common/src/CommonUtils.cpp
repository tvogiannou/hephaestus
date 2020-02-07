#include <common/CommonUtils.h>

#include <hephaestus/TriMeshPipeline.h>

#include <fstream>
#include <cassert>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


namespace hephaestus
{
template bool CommonUtils::ToString<int32_t>(const int32_t& , std::string& );
template bool CommonUtils::ToString<uint32_t>(const uint32_t& , std::string& );
template bool CommonUtils::ToString<float>(const float& , std::string& );
template bool CommonUtils::ToString<double>(const double& , std::string& );

template bool CommonUtils::FromString<int32_t>(const std::string&, int32_t&);
template bool CommonUtils::FromString<uint32_t>(const std::string&, uint32_t&);
template bool CommonUtils::FromString<float>(const std::string&, float&);
template bool CommonUtils::FromString<double>(const std::string&, double&);


static 
float
s_Deg2Rad(float degrees)
{
    static const float pi_over_180 = 4.f * std::atan(1.f) / 180.f;
    return degrees * pi_over_180;
}

static
void
s_SplitFilename(std::string& str)
{
    size_t found = str.find_last_of("/\\");
    if (found != std::string::npos)
        str = str.substr(0, found);
    else
        str = ".";  // set current directory if no slash character was found

    str.append({ '/' });
}

bool 
CommonUtils::LoadObjFile(const char* filename, MeshUtils::TriMesh& triMesh, OBJFileInfo& objInfo)
{
    triMesh.vertexData.clear();
    triMesh.indices.clear();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    // use the same path for the material file
    std::string materialPath(filename);
    s_SplitFilename(materialPath);

    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, materialPath.c_str());

    if (!ret)
        return false;

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++)
    {
        // TODO: support more shapes
        if (s > 1)
            break;

        const bool hasNormals = !attrib.normals.empty();
        const bool hasUVs = !attrib.texcoords.empty();
        const bool hasMaterial = materials.size() == 1; // only one material supported

        // reconstruct the material path
        if (hasMaterial)
            objInfo.textureFilename = materialPath + materials[0].diffuse_texname;
        
        objInfo.hasNormals = hasNormals;

        // copy vertex data first
        {
            size_t vertexIndex = 0;
            size_t texIndex = 0;
            triMesh.vertexCount = (uint32_t)(attrib.vertices.size() / 3);
            triMesh.vertexData.reserve(triMesh.vertexCount * 
                (sizeof(TriMeshPipeline::VertexData) / sizeof(float)));
            while (vertexIndex < attrib.vertices.size())
            {
                assert(!hasUVs || texIndex < attrib.texcoords.size());
                assert(!hasNormals || vertexIndex < attrib.normals.size());  // should use proper index in case normals do not match verts

                // load vertex data
                const tinyobj::real_t vx = attrib.vertices[vertexIndex + 0];
                const tinyobj::real_t vy = attrib.vertices[vertexIndex + 1];
                const tinyobj::real_t vz = attrib.vertices[vertexIndex + 2];
                const tinyobj::real_t nx = hasNormals ? attrib.normals[vertexIndex + 0] : 0.f;
                const tinyobj::real_t ny = hasNormals ? attrib.normals[vertexIndex + 1] : 0.f;
                const tinyobj::real_t nz = hasNormals ? attrib.normals[vertexIndex + 2] : 0.f;
                const tinyobj::real_t tx = hasUVs ? attrib.texcoords[texIndex + 0] : 0.f;
                const tinyobj::real_t ty = hasUVs ? attrib.texcoords[texIndex + 1] : 0.f;
                // Optional: vertex colors
                // tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
                // tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
                // tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];

                // write to output buffer
                triMesh.vertexData.push_back((float)vx);
                triMesh.vertexData.push_back((float)vy);
                triMesh.vertexData.push_back((float)vz);
                triMesh.vertexData.push_back((float)nx);
                triMesh.vertexData.push_back((float)ny);
                triMesh.vertexData.push_back((float)nz);
                triMesh.vertexData.push_back((float)tx);
                triMesh.vertexData.push_back((float)ty);

                if (hasMaterial)
                {
                    triMesh.vertexData.push_back(materials[0].diffuse[0]);
                    triMesh.vertexData.push_back(materials[0].diffuse[1]);
                    triMesh.vertexData.push_back(materials[0].diffuse[2]);
                }
                else
                {
                    // default color
                    float r = 1.f, g = 1.f, b = 1.f;
                    triMesh.vertexData.push_back(r);
                    triMesh.vertexData.push_back(g);
                    triMesh.vertexData.push_back(b);
                }

                // get next vertex
                vertexIndex += 3;
                texIndex += 2;
            }
        }

        // copy indices
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            const size_t fv = shapes[s].mesh.num_face_vertices[f];
            assert(fv == 3); // only support for triangulated faces

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++)
            {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                triMesh.indices.push_back((uint32_t)idx.vertex_index);
            }
            index_offset += fv;

            // per-face material
            //shapes[s].mesh.material_ids[f];
        }
    }

    return true;
}

bool 
CommonUtils::LoadBinaryFileContents(const char* filename, std::vector<char>& fileContents)
{
    std::ifstream file(filename, std::ios::binary);
    if (file.fail()) 
        return false;

    std::streampos begin = file.tellg();
    file.seekg(0, std::ios::end);
    std::streampos end = file.tellg();

    const size_t newSize = static_cast<size_t>(end - begin);
    fileContents.resize(newSize);

    file.seekg(0, std::ios::beg);
    file.read(fileContents.data(), end - begin);
    file.close();

    return true;
}

bool
CommonUtils::LoadImageFromFile(const char* filename, const ImageLoadInfo& info,
    std::vector<char>& imageData, MeshUtils::ImageDesc& imageDesc)
{
    std::vector<char> fileData;
    if (!LoadBinaryFileContents(filename, fileData))
        return false;

    return LoadImageFromBuffer(fileData.data(), (uint32_t)fileData.size(),
                        info, imageData, imageDesc);
}

bool 
CommonUtils::LoadImageFromBuffer(const char* rawDataBuffer, uint32_t size, const ImageLoadInfo& info,
    std::vector<char>& imageData, MeshUtils::ImageDesc& imageDesc)
{
    int tmp_width = 0, tmp_height = 0, tmp_components = 0;
    
    stbi_set_flip_vertically_on_load(info.flipVertically); // do we need this only for png?

    unsigned char *image_data = stbi_load_from_memory(
        reinterpret_cast<const unsigned char*>(rawDataBuffer),
        static_cast<int>(size),
        &tmp_width, &tmp_height,
        &tmp_components, info.requestedComponents);

    if ((image_data == nullptr) ||
        (tmp_width <= 0) ||
        (tmp_height <= 0) ||
        (tmp_components <= 0))
    {
        return false;
    }

    // copy data to output buffer
    const int imageSize = (tmp_width) * (tmp_height) *
        (info.requestedComponents == 0 ? tmp_components : info.requestedComponents);
    imageData.resize(imageSize);
    std::memcpy(imageData.data(), image_data, imageSize);

    stbi_image_free(image_data);

    // set the rest of the image desc properties
    imageDesc.width = tmp_width;
    imageDesc.height = tmp_height;
    //imageDesc.numComponents = tmp_components;
    imageDesc.numComponents = info.requestedComponents;

    return true;
}

bool 
CommonUtils::GetPerspectiveProjectionMatrixVulkan(float aspect_ratio, float field_of_view, 
                                                float near_clip, float far_clip, 
                                                Matrix4& projection)
{
    // Projection matrix corrected for Vulkan 
    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vertexpostproc-clipping
    // https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/

    const float f = 1.f / std::tan(0.5f * s_Deg2Rad(field_of_view));

    projection = Matrix4(
        f / aspect_ratio,
        0.0f,
        0.0f,
        0.0f,

        0.0f,
        -f,
        0.0f,
        0.0f,

        0.0f,
        0.0f,
        0.5f * (near_clip + far_clip) / (near_clip - far_clip),
        0.5f * (near_clip + far_clip + 4.f * near_clip * far_clip) / (near_clip - far_clip),

        0.0f,
        0.0f,
        -0.5f,
        -0.5f);

    // TODO: return false if bad params
    return true;
}

bool
CommonUtils::GetPerspectiveProjectionMatrixBasic(float aspect_ratio, float field_of_view, 
                                                float near_clip, float far_clip,
                                                Matrix4& projection)
{
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix

    const float f = 1.f / std::tan(0.5f * s_Deg2Rad(field_of_view));

    // row major
    projection = Matrix4(
        f / aspect_ratio,
        0.0f,
        0.0f,
        0.0f,

        0.0f,
        f,
        0.0f,
        0.0f,

        0.0f,
        0.0f,
        (near_clip + far_clip) / (near_clip - far_clip),
        -1.0f,

        0.0f,
        0.0f,
        (2.f * near_clip * far_clip) / (near_clip - far_clip),
        0.0f
    );

    // TODO: return false if bad params
    return true;
}

bool
CommonUtils::GetOrthographicProjectionMatrix(float left_plane, float right_plane, float top_plane,
                                             float bottom_plane, float near_plane, float far_plane,
                                             Matrix4& projection)
{
    // ref https://github.com/GameTechDev/IntroductionToVulkan/blob/master/Project/Common/Tools.cpp
    projection = Matrix4(
    	2.0f / (right_plane - left_plane),
    	0.0f,
    	0.0f,
    	0.0f,

    	0.0f,
    	2.0f / (bottom_plane - top_plane),
    	0.0f,
    	0.0f,

    	0.0f,
    	0.0f,
    	1.0f / (near_plane - far_plane),
    	0.0f,

    	-(right_plane + left_plane) / (right_plane - left_plane),
    	-(bottom_plane + top_plane) / (bottom_plane - top_plane),
    	near_plane / (near_plane - far_plane),
    	1.0f
    );

    // TODO: return false if bad params
    return true;
}

}