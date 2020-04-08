
#include <common/AxisAlignedBoundingBox.h>
#include <common/Camera.h>
#include <common/CommonUtils.h>
#include <common/MeshUtils.h>
#include <common/Matrix4.h>
#include <hephaestus/Log.h>
#include <hephaestus/Compiler.h>
#include <hephaestus/TriMeshPipeline.h>
#include <hephaestus/HeadlessRenderer.h>
#include <hephaestus/VulkanConfig.h>

#include <cstdlib>
#include <cassert>


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

enum ShaderType
{
    eSHADER_VERTEX_PNTC = 0,
    eSHADER_FRAGMENT_PhongNoTexture = 1,
    eSHADER_FRAGMENT_PhongTexture = 2
};


hephaestus::VulkanDispatcher::ModuleType s_vulkanLib = (hephaestus::VulkanDispatcher::ModuleType)nullptr;
static void UnloadVulkanLib()
{
#ifdef HEPHAESTUS_PLATFORM_WIN32
    if (s_vulkanLib)
        FreeLibrary(s_vulkanLib);
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
    if (s_vulkanLib)
        dlclose(s_vulkanLib);
#endif
}

#define CHECK_EXIT_MSG(expr, msg)	\
if (!(expr)) { HEPHAESTUS_LOG_ERROR(msg); std::exit(EXIT_FAILURE); }


int main()
{
    // Load the Vulkan dynamic lib
    {
#ifdef HEPHAESTUS_PLATFORM_WIN32
        s_vulkanLib = LoadLibrary("vulkan-1.dll");
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
        s_vulkanLib = dlopen("libvulkan.so.1", RTLD_NOW);
#endif
        if (s_vulkanLib == nullptr)
            std::exit(EXIT_FAILURE);

        std::atexit(UnloadVulkanLib);
    }

    // create the dispatcher for the loaded Vulkan functions
    {
        hephaestus::VulkanDispatcher::InitFromLibrary(s_vulkanLib);
        hephaestus::VulkanDispatcher::LoadGlobalFunctions();
    }

    // create the device manager
    hephaestus::VulkanDeviceManager deviceManager;
    {
        // setup the manager for headless rendering
        constexpr bool enableValidationLayers = true;
        hephaestus::VulkanDeviceManager::PlatformWindowInfo windowInfo = {}; // null window
        CHECK_EXIT_MSG(deviceManager.Init(windowInfo, enableValidationLayers),
            "Failed to initialize Vulkan device manager");
    }

    //hephaestus::MallocAllocator allocator;
    constexpr uint32_t outWidth = 1024u;
    constexpr uint32_t outHeight = 1024u;

    // init headless renderer
    hephaestus::HeadlessRenderer renderer(deviceManager);
    {
        hephaestus::HeadlessRenderer::InitInfo info = {};
        info.width = outWidth;
        info.height = outHeight;
        CHECK_EXIT_MSG(renderer.Init(info),"Failed to init headless renderer");
    }

    hephaestus::VulkanUtils::ShaderDB shaderDB;
    {
        shaderDB.loadedShaders[ShaderType::eSHADER_VERTEX_PNTC] =
            hephaestus::VulkanUtils::CreateShaderModule(deviceManager, "../data/shaders/mesh/mesh.vert.spv");
        shaderDB.loadedShaders[ShaderType::eSHADER_FRAGMENT_PhongNoTexture] =
            hephaestus::VulkanUtils::CreateShaderModule(deviceManager, "../data/shaders/mesh/mesh_notexture.frag.spv");
        shaderDB.loadedShaders[ShaderType::eSHADER_FRAGMENT_PhongTexture] =
            hephaestus::VulkanUtils::CreateShaderModule(deviceManager, "../data/shaders/mesh/mesh.frag.spv");
//         shaderDB.loadedShaders[ShaderType::eSHADER_VERTEX_Lines] =
//             VulkanUtils::CreateShaderModule(m_deviceManager, std::string(dirStr + "/primitives/lines.vert.spv").c_str());
//         shaderDB.loadedShaders[ShaderType::eSHADER_FRAGMENT_Lines] =
//             VulkanUtils::CreateShaderModule(m_deviceManager, std::string(dirStr + "/primitives/lines.frag.spv").c_str()    
    }

    // load data into pipeline
    hephaestus::TriMeshPipeline meshPipeline(renderer.GetDeviceManager());
    {
        //const char* filename = "../data/bunny.obj";
        const char* filename = "../data/teapot.obj";
        hephaestus::MeshUtils::TriMesh mesh;
        hephaestus::CommonUtils::OBJFileInfo objInfo;
        CHECK_EXIT_MSG(hephaestus::CommonUtils::LoadObjFile(filename, mesh, objInfo), "Failed to load file");

        if (!objInfo.hasNormals)
            hephaestus::MeshUtils::ComputeSmoothNormals(mesh);

        // TODO: use loaded texture
        // dummy texture
        std::vector<char> textureData(16, '\xfa');
        hephaestus::MeshUtils::ImageDesc textureDesc = { 2, 2, 4 };
        // shaders to be used by the pipeline, need to match vertex format
        hephaestus::PipelineBase::ShaderParams shaderParams(shaderDB);
        {
            shaderParams.vertexShaderIndex = ShaderType::eSHADER_VERTEX_PNTC;
            shaderParams.fragmentShaderIndex = ShaderType::eSHADER_FRAGMENT_PhongTexture;
        }
        hephaestus::TriMeshPipeline::SetupParams params = {}; // default pipeline params
        CHECK_EXIT_MSG(hephaestus::MeshUtils::SetupPipelineForMesh(
            mesh, textureData, textureDesc, renderer.GetCmdBuffer(), renderer.GetRenderPass(), 
            shaderParams, params, meshPipeline), 
            "Failed to setup pipeline");

        // set the camera & transform
        {
            std::array<float, 16> matrixData;

            // compute perspective projection
            const float aspectRatio = ((float)outWidth) / ((float)outHeight);
            const float fov = 60.f;
            hephaestus::Matrix4 projection;
            hephaestus::CommonUtils::GetPerspectiveProjectionMatrixVulkan(aspectRatio, fov, 0.1f, 100.f, projection);

            projection.GetRaw(matrixData);
            CHECK_EXIT_MSG(meshPipeline.UpdateProjectionMatrix(matrixData, renderer.GetCmdBuffer()),
                "Failed to update projection matrix");

            // compute look at camera from the bbox of the data
            hephaestus::AxisAlignedBoundingBox bbox = hephaestus::MeshUtils::ComputeBoundingBox(mesh);
            hephaestus::Vector3 center = bbox.ComputeCenter();
            hephaestus::Vector3 camPos = bbox.max;
            camPos.y = center.y;	// move to about the mid axis of the face
            camPos.Mul(3.f);		// move a bit further away

            hephaestus::Camera camera;
            camera.SetLookAt(camPos, center);
            hephaestus::Matrix4 view;
            camera.GetViewRenderMatrix(view);

            view.GetRaw(matrixData);
            CHECK_EXIT_MSG(meshPipeline.UpdateViewMatrix(matrixData, renderer.GetCmdBuffer()),
                "Failed to update view matrix");

            CHECK_EXIT_MSG(meshPipeline.UpdateLightPos(
                { { 0.0f, 1.0f, 5.0f, 1.0f } }, renderer.GetCmdBuffer()), "Failed to update light position");

            // set transform for first mesh
            hephaestus::Matrix4 model; model.SetIdentity();
            model.GetRaw(matrixData);
            CHECK_EXIT_MSG(meshPipeline.UpdateModelMatrix(0u, matrixData, renderer.GetCmdBuffer()),
                "Failed to update mesh model transform");
        }
    }

    // render the pipeline
    CHECK_EXIT_MSG(renderer.RenderPipeline(meshPipeline), "Failed to render pipeline");

    // save the rendered image to a file
    {
        const char* filename = "renderedFrame.jpg";

        uint32_t numChannels = 0u;
        uint32_t width = 0u;
        uint32_t height = 0u;
        renderer.GetDstImageInfo(numChannels, width, height);

        char* imgData = reinterpret_cast<char*>(STBIW_MALLOC(numChannels * width * height));
        CHECK_EXIT_MSG(renderer.GetDstImageData(imgData), "Failed to read rendered image data");

        stbi_write_jpg(filename, width, height, numChannels, imgData, 100);

        STBIW_FREE(imgData);
    }

    renderer.Clear();


    return EXIT_SUCCESS;
}