#include "module.h"

#include <common/Camera.h>
#include <common/Matrix4.h>
#include <hephaestus/Platform.h>

#include <string>


// helper macro to check return values & throw an exception if fail
#define CHECK_EXIT_MSG(expr, msg)	\
if (!(expr)) { throw std::runtime_error(msg); }

// module global data
namespace hephaestus_bindings_globals
{

enum ShaderType
{
    eSHADER_VERTEX_PNTC = 0,
    eSHADER_FRAGMENT_PhongTexture
};

hephaestus::VulkanDispatcher::ModuleType s_vulkanLib = (hephaestus::VulkanDispatcher::ModuleType)nullptr;
hephaestus::VulkanDispatcher s_dispatcher;

// container so that we can explicitly delete the data before exit()
struct VulkanSystemInfo
{
    hephaestus::VulkanDeviceManager deviceManager;
    hephaestus::HeadlessRenderer renderer;
    hephaestus::TriMeshPipeline meshPipeline;
    hephaestus::VulkanUtils::ShaderDB shaderDB;

    static VulkanSystemInfo& GetInstance()
    {
        if (!s_instance)
            s_instance = new VulkanSystemInfo(hephaestus_bindings_globals::s_dispatcher);

        return *s_instance;
    }

    static void Destroy()
    {
        if (s_instance)
        {
            delete s_instance;
            s_instance = nullptr;
        }
    }

// "local" singleton, no automatic release
private:
    VulkanSystemInfo(hephaestus::VulkanDispatcher& _dispatcher) :
        deviceManager(_dispatcher),
        renderer(deviceManager),
        meshPipeline(deviceManager)
    {}

    static VulkanSystemInfo* s_instance;
};

VulkanSystemInfo* VulkanSystemInfo::s_instance = nullptr;
} // namespace hephaestus_bindings_globals

static 
void s_UnloadVulkanLib()
{
    using namespace hephaestus_bindings_globals;

#ifdef HEPHAESTUS_PLATFORM_WIN32
    if (s_vulkanLib)
    {
        FreeLibrary(s_vulkanLib);
        s_vulkanLib = (hephaestus::VulkanDispatcher::ModuleType)nullptr;
    }
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
    if (s_vulkanLib)
    {
        dlclose(s_vulkanLib);
        s_vulkanLib = (hephaestus::VulkanDispatcher::ModuleType)nullptr;
    }
#endif
}

void
HEPHAESTUS_BINDINGS_Init(uint32_t width, uint32_t height, const std::string& shaderDir)
{
    using namespace hephaestus_bindings_globals;

    // Load the Vulkan dynamic lib
    {
#ifdef HEPHAESTUS_PLATFORM_WIN32
        s_vulkanLib = LoadLibrary("vulkan-1.dll");
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
        s_vulkanLib = dlopen("libvulkan.so.1", RTLD_NOW);
#endif
        if (s_vulkanLib == nullptr)
            std::exit(EXIT_FAILURE);

        std::atexit(s_UnloadVulkanLib);
    }

    // create the dispatcher for the loaded Vulkan functions
    {
        s_dispatcher.InitFromLibrary(s_vulkanLib);
        s_dispatcher.LoadGlobalFunctions();
    }

    // create the instance with the system info
    VulkanSystemInfo& instance = VulkanSystemInfo::GetInstance();

    // create the device manager
    {
        // setup the manager for headless rendering
        bool enableValidationLayers = false;
        CHECK_EXIT_MSG(instance.deviceManager.Init({}, enableValidationLayers),
            "Failed to init device manager");
    }

    // init headless renderer
    {
        hephaestus::HeadlessRenderer::InitInfo info = {};
        info.width = width;
        info.height = height;
        CHECK_EXIT_MSG(instance.renderer.Init(info), "Failed to init headless renderer");
    }

    // init shader DB
    {
        instance.shaderDB.loadedShaders[ShaderType::eSHADER_VERTEX_PNTC] =
            hephaestus::VulkanUtils::CreateShaderModule(instance.deviceManager, 
                (shaderDir + std::string("/mesh/mesh.vert.spv")).c_str());
        instance.shaderDB.loadedShaders[ShaderType::eSHADER_FRAGMENT_PhongTexture] =
            hephaestus::VulkanUtils::CreateShaderModule(instance.deviceManager, 
                (shaderDir + std::string("/mesh/mesh.frag.spv")).c_str());
    }
}

void
HEPHAESTUS_BINDINGS_Clear()
{
    using namespace hephaestus_bindings_globals;
    VulkanSystemInfo& instance = VulkanSystemInfo::GetInstance();

    instance.deviceManager.WaitDevice();

    // let the destructor deal with the release of the Vulkan handles
    // make sure that no dynamically allocated Vulkan data are still alive
    // before we unload the Vulkan shared lib 
    VulkanSystemInfo::Destroy();

    s_UnloadVulkanLib();
}

void 
HEPHAESTUS_BINDINGS_SetClearColor(float r, float g, float b, float a)
{
    using namespace hephaestus_bindings_globals;
    VulkanSystemInfo& instance = VulkanSystemInfo::GetInstance();

    std::array<float, 4> colorValues = { r, g, b, a };
    instance.renderer.SetClearColor(colorValues);
}

void
HEPHAESTUS_BINDINGS_SetupForModel(const hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData& model)
{
    using namespace hephaestus_bindings_globals;
    VulkanSystemInfo& instance = VulkanSystemInfo::GetInstance();

    hephaestus::PipelineBase::ShaderParams shaderParams(instance.shaderDB);
    {
        shaderParams.vertexShaderIndex = ShaderType::eSHADER_VERTEX_PNTC;
        shaderParams.fragmentShaderIndex = ShaderType::eSHADER_FRAGMENT_PhongTexture;
    }
    hephaestus::TriMeshPipeline::SetupParams params = {};
    CHECK_EXIT_MSG(hephaestus::MeshUtils::SetupPipelineForMesh(
        model.m_trimesh, model.m_texture.data, model.m_texture.desc, 
        instance.renderer.GetCmdBuffer(), instance.renderer.GetRenderPass(),
        shaderParams, params, instance.meshPipeline),
        "Failed to setup pipeline");
}

void 
HEPHAESTUS_BINDINGS_SetOrthographicProjection(hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData& model,
    float left_plane, float right_plane, float top_plane,
    float bottom_plane, float near_plane, float far_plane)
{
    hephaestus::Matrix4 projection;
    hephaestus::CommonUtils::GetOrthographicProjectionMatrix(
        left_plane, right_plane, top_plane, bottom_plane, near_plane, far_plane, projection);

    projection.GetRaw(model.m_projectionMatrix);
}

void
HEPHAESTUS_BINDINGS_SetPerpespectiveProjection(hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData& model,
    float aspectRatio, float fov, float nearClip, float farClip)
{
    hephaestus::Matrix4 projection;
    hephaestus::CommonUtils::GetPerspectiveProjectionMatrixVulkan(
        aspectRatio, fov, nearClip, farClip, projection);

    projection.GetRaw(model.m_projectionMatrix);
}

void
HEPHAESTUS_BINDINGS_SetProjectionMatrix(hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData& model,
    const hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4& col0,
    const hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4& col1,
    const hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4& col2,
    const hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4& col3)
{
    hephaestus::Matrix4 m(
        col0.x, col0.y, col0.z, col0.w,
        col1.x, col1.y, col1.z, col1.w,
        col2.x, col2.y, col2.z, col2.w,
        col3.x, col3.y, col3.z, col3.w);

    m.GetRaw(model.m_projectionMatrix);
}

void HEPHAESTUS_BINDINGS_SetCameraLookAt(hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData& model,
    const hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4& cameraPos,
    const hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4& targetPos)
{
    const hephaestus::Vector3 camPos(cameraPos.x, cameraPos.y, cameraPos.z);
    const hephaestus::Vector3 target(targetPos.x, targetPos.y, targetPos.z);
    
    hephaestus::Camera camera;
    camera.SetLookAt(camPos, target);
    hephaestus::Matrix4 view;
    camera.GetViewRenderMatrix(view);

    view.GetRaw(model.m_modelviewMatrix);
}

void 
HEPHAESTUS_BINDINGS_SetModelViewMatrix(hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData& model,
    const hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4& col0,
    const hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4& col1,
    const hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4& col2,
    const hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4& col3)
{
    hephaestus::Matrix4 m(
        col0.x, col0.y, col0.z, col0.w,
        col1.x, col1.y, col1.z, col1.w,
        col2.x, col2.y, col2.z, col2.w,
        col3.x, col3.y, col3.z, col3.w);

    m.GetRaw(model.m_modelviewMatrix);
}

void
HEPHAESTUS_BINDINGS_UpdateLightPosition(hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData& /*mesh*/,
    const hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4& lightPos)
{
    using namespace hephaestus_bindings_globals;
    VulkanSystemInfo& instance = VulkanSystemInfo::GetInstance();

    std::array<float, 4> rawData;
    rawData[0] = lightPos.x;
    rawData[1] = lightPos.y;
    rawData[2] = lightPos.z;
    rawData[3] = lightPos.w;
    CHECK_EXIT_MSG(instance.meshPipeline.UpdateLightPos(
        rawData, instance.renderer.GetCmdBuffer()),
        "Failed to update the light position");
}

// returns (image data, number of channels, width, height)
std::tuple<pybind11::array_t<uint8_t>, uint32_t, uint32_t, uint32_t>
HEPHAESTUS_BINDINGS_RenderMesh(const hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData& model)
{
    using namespace hephaestus_bindings_globals;
    VulkanSystemInfo& instance = VulkanSystemInfo::GetInstance();

    CHECK_EXIT_MSG(instance.meshPipeline.UpdateViewAndProjectionMatrix(
        model.m_modelviewMatrix, model.m_projectionMatrix, instance.renderer.GetCmdBuffer()),
        "Failed to update view and projection matrices");

    // render the pipeline
    CHECK_EXIT_MSG(instance.renderer.RenderPipeline(instance.meshPipeline), "Failed to render pipeline");

    return hephaestus_bindings::Utils::ExtractRendererDstImage(instance.renderer);
}

pybind11::array_t<uint8_t>
HEPHAESTUS_BINDINGS_RenderMeshOnImage(const hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData& model,
    pybind11::array_t<uint8_t> dstImage)
{
    using namespace hephaestus_bindings_globals;
    VulkanSystemInfo& instance = VulkanSystemInfo::GetInstance();

    CHECK_EXIT_MSG(instance.meshPipeline.UpdateViewAndProjectionMatrix(
        model.m_modelviewMatrix, model.m_projectionMatrix, instance.renderer.GetCmdBuffer()),
        "Failed to update view and projection matrices");

    // render the pipeline
    CHECK_EXIT_MSG(instance.renderer.RenderPipeline(instance.meshPipeline), "Failed to render pipeline");

    // combine the output with the input image
    // get the renderer target image dimensions
    uint32_t renderNumChannels = 0u;
    uint32_t renderWidth = 0u;
    uint32_t renderHeight = 0u;
    instance.renderer.GetDstImageInfo(renderNumChannels, renderWidth, renderHeight);
    const size_t renderSize = renderNumChannels * renderWidth * renderHeight;

    // get the destination image dimensions and make sure they match the renderer target
    pybind11::buffer_info dstBufferInfo = dstImage.request();
    if (dstBufferInfo.ndim != 3)
        throw std::runtime_error("Invalid destination image shape size, should be 3 (h, w, RGB)");

    const uint32_t dstHeight = (uint32_t)dstBufferInfo.shape[0];
    const uint32_t dstWidth = (uint32_t)dstBufferInfo.shape[1];
    const uint32_t dstNumChannels = (uint32_t)dstBufferInfo.shape[2];
    if (dstHeight != renderHeight || dstWidth != renderWidth || dstNumChannels != renderNumChannels)
        throw std::runtime_error("Invalid destination image shape, should match renderer target width & height");

    // allocate normal buffer
    auto result = pybind11::array_t<uint8_t>(renderSize);
    pybind11::buffer_info resultBufferInfo = result.request();
    char* resImageData = (char*)resultBufferInfo.ptr;

    CHECK_EXIT_MSG(instance.renderer.GetDstImageData(resImageData), "Failed to read rendered image data");

    // combine images
    char* dstData = (char*)dstBufferInfo.ptr;
    {
        // get the color key as the clear color
        hephaestus::RendererBase::Color4 clearColor = instance.renderer.GetClearColor();
        const std::array<char, 3> colorKey = {{
            (char)(clearColor[0] * 255.f),   // convert RGB to char values 
            (char)(clearColor[1] * 255.f),
            (char)(clearColor[2] * 255.f),
        }};

        for (uint32_t i = 0; i < renderSize; i += renderNumChannels)
        {
            // TODO: use loop over num?
            if (resImageData[i] == colorKey[0] &&
                resImageData[i + 1] == colorKey[1] &&
                resImageData[i + 2] == colorKey[2])
            {
                resImageData[i]     = dstData[i];
                resImageData[i + 1] = dstData[i + 1];
                resImageData[i + 2] = dstData[i + 2];
            }
        }
    }

    return result;
}

hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData
HEPHAESTUS_BINDINGS_CreateMeshFromData(
    pybind11::array_t<float, pybind11::array::c_style | pybind11::array::forcecast> vertices,
    pybind11::array_t<uint32_t> indices)
{
    hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData model;

    // dummy texture
    {
        model.m_texture.data.resize(16, '\xfa');
        model.m_texture.desc.width = 2u;
        model.m_texture.desc.height = 2u;
        model.m_texture.desc.numComponents = 4u;
    }

    // create mesh vertex data
    {
        pybind11::buffer_info vertexBufferInfo = vertices.request();
        pybind11::buffer_info indexBufferInfo = indices.request();

        if (vertexBufferInfo.ndim != 1u || indexBufferInfo.ndim != 1u)
            throw std::runtime_error("Number of dimensions must be one");

        const float* vtx = (const float*)vertexBufferInfo.ptr;
        const uint32_t* idx = (const uint32_t*)indexBufferInfo.ptr;

        const size_t numVertices = vertexBufferInfo.shape[0];
        const size_t numIndices = indexBufferInfo.shape[0];
        // const size_t vertexCount = numVertices / 3u;
        // const size_t faceCount = numIndices / 3u;

        CHECK_EXIT_MSG(hephaestus_bindings::Utils::CopyTriMeshToRenderMeshData(
            vtx, numVertices, idx, numIndices, nullptr, model.m_trimesh), 
            "Failed to convert input tri mesh data");
    }

    hephaestus::MeshUtils::ComputeSmoothNormals(model.m_trimesh);

    return model;
}

hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData
HEPHAESTUS_BINDINGS_CreateUVMeshFromData(
    pybind11::array_t<float, pybind11::array::c_style | pybind11::array::forcecast> vertices,
    pybind11::array_t<uint32_t> indices,
    pybind11::array_t<float, pybind11::array::c_style | pybind11::array::forcecast> uv)
{
    hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData model;

    // dummy texture
    {
        model.m_texture.data.resize(16, '\xfa');
        model.m_texture.desc.width = 2u;
        model.m_texture.desc.height = 2u;
        model.m_texture.desc.numComponents = 4u;
    }

    // create mesh vertex data
    {
        pybind11::buffer_info vertexBufferInfo = vertices.request();
        pybind11::buffer_info indexBufferInfo = indices.request();
        pybind11::buffer_info uvBufferInfo = uv.request();

        if (vertexBufferInfo.ndim != 1u || indexBufferInfo.ndim != 1u || uvBufferInfo.ndim != 1u)
            throw std::runtime_error("Number of dimensions must be one");

        const float* vtx = (const float*)vertexBufferInfo.ptr;
        const uint32_t* idx = (const uint32_t*)indexBufferInfo.ptr;
        const float* uvData = (const float*)uvBufferInfo.ptr;

        const size_t numVertices = vertexBufferInfo.shape[0];
        const size_t numIndices = indexBufferInfo.shape[0];
        const size_t vertexCount = numVertices / 3u;
        // const size_t faceCount = numIndices / 3u;

        const size_t numUVs = uvBufferInfo.shape[0] / 2u;
        if (numUVs != vertexCount)
            throw std::runtime_error("Number of UV should match the number of vertices");

        CHECK_EXIT_MSG(hephaestus_bindings::Utils::CopyTriMeshToRenderMeshData(
            vtx, numVertices, idx, numIndices, uvData, model.m_trimesh),
            "Failed to convert input tri mesh data");
    }

    hephaestus::MeshUtils::ComputeSmoothNormals(model.m_trimesh);

    return model;
}

void
HEPHAESTUS_BINDINGS_UpdateMeshPositions(
    hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData& model,
    pybind11::array_t<float, pybind11::array::c_style | pybind11::array::forcecast> positions,
    bool updateNormals = true)
{
    using namespace hephaestus;

    if (model.m_trimesh.vertexData.empty())
        throw std::runtime_error("Invalid model");

    // write new positions to mesh vertex buffer
    {
        pybind11::buffer_info posBufferInfo = positions.request();
        if (posBufferInfo.ndim != 1u)
            throw std::runtime_error("Number of dimensions must be one");

        const float* positionsBuffer = (const float*)posBufferInfo.ptr;
        const size_t numPositions = posBufferInfo.shape[0];

        if (numPositions * 3u != model.m_trimesh.vertexCount)
            throw std::runtime_error("Size of positions does not match model vertices");

        const size_t step = sizeof(TriMeshPipeline::VertexData) / sizeof(float);
        for (size_t i = 0u; i < model.m_trimesh.vertexData.size(); i += step)
        {
            model.m_trimesh.vertexData[i] = *positionsBuffer++;
            model.m_trimesh.vertexData[i + 1] = *positionsBuffer++;
            model.m_trimesh.vertexData[i + 2] = *positionsBuffer++;
        }
    }

    if (updateNormals)
        MeshUtils::ComputeSmoothNormals(model.m_trimesh);

    // update vertex buffer
    {
        using namespace hephaestus_bindings_globals;
        VulkanSystemInfo& instance = VulkanSystemInfo::GetInstance();

        VulkanUtils::BufferUpdateInfo updateInfo;
        {
            updateInfo.copyCmdBuffer = instance.renderer.GetCmdBuffer();
            updateInfo.data = reinterpret_cast<const char*>(model.m_trimesh.vertexData.data());
            updateInfo.dataSize = (uint32_t)(model.m_trimesh.vertexData.size() * sizeof(float));
        }

        if (!VulkanUtils::CopyBufferDataHost(
            instance.deviceManager, updateInfo, instance.meshPipeline.GetVertexBufferInfo()))
            throw std::runtime_error("Failed to update vertex data");
    }
}

void
HEPHAESTUS_BINDINGS_SetMeshTexture(
    hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData& model,
    pybind11::array_t<char, pybind11::array::c_style | pybind11::array::forcecast> texture)
{
    using namespace hephaestus;

    pybind11::buffer_info textureBufferInfo = texture.request();

    if (textureBufferInfo.ndim != 3u)
        throw std::runtime_error("Number of texture dimensions must be 3");

    model.m_texture.desc.height = (uint32_t)textureBufferInfo.shape[0];
    model.m_texture.desc.width = (uint32_t)textureBufferInfo.shape[1];
    model.m_texture.desc.numComponents = (uint32_t)textureBufferInfo.shape[2];

    if (model.m_texture.desc.numComponents != 4u)
        throw std::runtime_error("Texture should be RGBA");
 
    const char* imgData = (const char*)textureBufferInfo.ptr;
    model.m_texture.data = std::vector<char>(imgData, imgData + textureBufferInfo.size);
}

std::tuple<pybind11::array_t<float>, pybind11::array_t<uint32_t>>
HEPHAESTUS_BINDINGS_LoadObjFile(const std::string& filename)
{
    using namespace hephaestus;

    hephaestus::MeshUtils::TriMesh mesh;
    hephaestus::CommonUtils::OBJFileInfo objInfo;
    CHECK_EXIT_MSG(hephaestus::CommonUtils::LoadObjFile(
        filename.c_str(), mesh, objInfo), "Failed to load OBJ file");
     
    // allocate return buffers
    auto verticesBuffer = pybind11::array_t<float>(mesh.vertexCount * 3u);
    auto indicesBuffer = pybind11::array_t<uint32_t>(mesh.indices.size());
    pybind11::buffer_info verticesBufferInfo = verticesBuffer.request();
    pybind11::buffer_info indicesBufferInfo = indicesBuffer.request();

    // copy to output buffers
    std::memcpy(indicesBufferInfo.ptr, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));

    const uint32_t step = sizeof(TriMeshPipeline::VertexData) / sizeof(float);
    float* dst = verticesBuffer.mutable_data();
    for (size_t i = 0u; i < mesh.vertexData.size(); i += step)
    {
        *dst++ = mesh.vertexData[i];
        *dst++ = mesh.vertexData[i + 1];
        *dst++ = mesh.vertexData[i + 2];
    }

    return std::make_tuple(verticesBuffer, indicesBuffer);
}


// python module definition
PYBIND11_MODULE(hephaestus_bindings, m)
{
    // helper struct to hold XYZW vectors
    pybind11::class_<hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4>(m, "vec4")
        .def(pybind11::init<float, float, float, float>(),
            pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("z"), pybind11::arg("w"))
        .def_readwrite("x", &hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4::x)
        .def_readwrite("y", &hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4::y)
        .def_readwrite("z", &hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4::z)
        .def_readwrite("w", &hephaestus_bindings::HEPHAESTUS_BINDINGS_Vec4::w);

    // model storing all the data to be able to render & update the NMFCs of a mesh
    pybind11::class_<hephaestus_bindings::HEPHAESTUS_BINDINGS_MeshData>(m, "Mesh");

    m.def("create_mesh", &HEPHAESTUS_BINDINGS_CreateMeshFromData, "Create a mesh from the input vertex & index data",
        pybind11::arg("vertices"), pybind11::arg("indices"));
    m.def("create_uv_mesh", &HEPHAESTUS_BINDINGS_CreateUVMeshFromData, "Create a mesh with UVs from the input vertex & index data",
        pybind11::arg("vertices"), pybind11::arg("indices"), pybind11::arg("uv"));
    m.def("update_mesh_positions", &HEPHAESTUS_BINDINGS_UpdateMeshPositions, "Update the vertex positions of a mesh",
        pybind11::arg("mesh"), pybind11::arg("positions"), pybind11::arg("update_normals") = true);
    m.def("setup_model", &HEPHAESTUS_BINDINGS_SetupForModel, "Setup the pipeline for the mesh model",
        pybind11::arg("model"));

    // system wide methods
    m.def("init_system", &HEPHAESTUS_BINDINGS_Init, "Initialize the system, loading the Vulkan dll and creating global vars",
        pybind11::arg("width"), pybind11::arg("height"), pybind11::arg("shaderDir"));
    m.def("clear_system", &HEPHAESTUS_BINDINGS_Clear, "Release all memory allocated by the system and close the Vulkan dll");
    m.def("set_clear_color", &HEPHAESTUS_BINDINGS_SetClearColor, 
        "Set the clear color of the renderer, i.e. the color of the pixels with no geometry",
        pybind11::arg("r"), pybind11::arg("g"), pybind11::arg("b"), pybind11::arg("a"));

    // render methods
    m.def("render_mesh", &HEPHAESTUS_BINDINGS_RenderMesh, "Render the mesh",
        pybind11::arg("model"));
    
    // projection matrix methods
    m.def("set_perspective_projection", &HEPHAESTUS_BINDINGS_SetPerpespectiveProjection, 
        "Set the projection as a perspective matrix directly",
        pybind11::arg("model"), 
        pybind11::arg("aspectRatio"), pybind11::arg("fov"), pybind11::arg("near"), pybind11::arg("far"));
    m.def("set_orthographics_projection", &HEPHAESTUS_BINDINGS_SetOrthographicProjection,
        "Set the projection as a perspective matrix directly",
        pybind11::arg("model"),
        pybind11::arg("left_plane"), pybind11::arg("right_plane"), 
        pybind11::arg("top_plane"), pybind11::arg("bottom_plane"),
        pybind11::arg("near_plane"), pybind11::arg("far_plane"));
    m.def("set_projection", &HEPHAESTUS_BINDINGS_SetProjectionMatrix, "Set the projection matrix directly",
        pybind11::arg("model"),
        pybind11::arg("col0"), pybind11::arg("col1"), pybind11::arg("col2"), pybind11::arg("col3"));
    
    // modelview matrix methods    
    m.def("set_camera_lookat", &HEPHAESTUS_BINDINGS_SetCameraLookAt,
        "Set the model view matrix from a look-at camera",
        pybind11::arg("model"), pybind11::arg("camera_pos"), pybind11::arg("camera_target"));
    m.def("set_modelview", &HEPHAESTUS_BINDINGS_SetModelViewMatrix, "Set the model view matrix",
        pybind11::arg("model"),
        pybind11::arg("col0"), pybind11::arg("col1"), pybind11::arg("col2"), pybind11::arg("col3"));

    // utils
    m.def("set_light_pos", &HEPHAESTUS_BINDINGS_UpdateLightPosition,
        "Set the position of the light used in the shader for the mesh pipeline",
        pybind11::arg("model"), pybind11::arg("light_pos"));
    m.def("render_mesh_image", &HEPHAESTUS_BINDINGS_RenderMeshOnImage, "Render the mesh onto the input image; input is expected to be RGBA",
        pybind11::arg("model"), pybind11::arg("image"));
    m.def("load_obj", &HEPHAESTUS_BINDINGS_LoadObjFile, "Load the vertices & indices of the input OBJ file",
        pybind11::arg("filename"));
    m.def("set_mesh_texture", &HEPHAESTUS_BINDINGS_SetMeshTexture,
        "Set the texture for this mesh. Requires to have been created with UV data",
        pybind11::arg("model"), pybind11::arg("texture"));
}
