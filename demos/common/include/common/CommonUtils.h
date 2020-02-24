#pragma once

#include <common/Matrix4.h>
#include <common/MeshUtils.h>

#include <array>
#include <string>
#include <sstream>
#include <type_traits>
#include <vector>


namespace hephaestus
{
// Collection of generic tools used by the demos
// These are low effort implementations suited for the needs of this project
// and not intended to be designed as generic scalable solutions
struct CommonUtils
{
    struct ImageLoadInfo
    {
        uint32_t requestedComponents;
        bool flipVertically = true;
    };

    struct OBJFileInfo
    {
        std::string textureFilename;
        bool hasNormals;
    };

    // utils for loading data from files
    static bool LoadBinaryFileContents(const char* filename, std::vector<char>& fileContents);
    // reads the OBJ file and stores the geometry into a triangular mesh with the VulkanMeshGraphicsPipeline 
    // vertex format
    static bool LoadObjFile(const char* filename, MeshUtils::TriMesh& triMesh, OBJFileInfo& objInfo);
    static bool LoadImageFromFile(const char* filename, const ImageLoadInfo& info, 
                                std::vector<char>& imageData, MeshUtils::ImageDesc& imageDesc);
    static bool LoadImageFromBuffer(const char* rawDataBuffer, uint32_t size, const ImageLoadInfo& info,
                                    std::vector<char>& imageData, MeshUtils::ImageDesc& imageDesc);

    // projection matrix utils
    static bool GetPerspectiveProjectionMatrixVulkan(float aspect_ratio, float field_of_view, 
                                                     float near_clip, float far_clip,
                                                     Matrix4& projection);
    static bool GetPerspectiveProjectionMatrixBasic(float aspect_ratio, float field_of_view, 
                                                    float near_clip, float far_clip,
                                                    Matrix4& projection);
    static bool GetOrthographicProjectionMatrix(float left_plane, float right_plane, float top_plane,
                                                float bottom_plane, float near_plane, float far_plane,
                                                Matrix4& projection);

    // string utils
    template<typename Type>
    static bool FromString(const std::string& str, Type& out)
    {
        static_assert(std::is_pod<Type>::value, "FromString: only POD types are supported");

        std::istringstream iss(str);
        iss >> out;

        return !iss.fail();
    }

    template<typename Type>
    static bool ToString(const Type& in, std::string& str)
    {
        static_assert(std::is_pod<Type>::value, "FromString: only POD types are supported");

        std::ostringstream oss;
        oss << in;
        str = oss.str();    // can we just back oss with the input str?

        return !oss.fail();
    }
};

}