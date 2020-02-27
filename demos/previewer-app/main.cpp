
#include <hephaestus/Log.h>
#include <common/Camera.h>
#include <common/CommonUtils.h>
#include <hephaestus/VulkanDeviceManager.h>
#include <hephaestus/TriMeshPipeline.h>
#include "SimpleWindow.h"
#include "WindowRenderer.h"

#include <imgui.h>
#include "imgui_impl_vulkan.h"


// Window system config
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef HEPHAESTUS_PLATFORM_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>
#include "imgui_impl_glfw.h"


enum ShaderType
{
    eSHADER_VERTEX_PNTC = 0,
    eSHADER_FRAGMENT_PhongNoTexture,
    eSHADER_FRAGMENT_PhongTexture,
    eSHADER_VERTEX_Lines,
    eSHADER_FRAGMENT_Lines

};

class WindowUpdater : public hephaestus::SimpleWindow::Updater
{
public:

    struct UIState
    {
        enum CameraMode : int
        {
            eCAMERA_LOOKAT = 0,
            eCAMERA_FREE = 1
        };

        // camera state
        CameraMode cameraMode = eCAMERA_LOOKAT;
        float cameraMoveStep = 0.005f;
        float cameraRotateStep = 0.1f;
        bool cameraNeedsReset = false;

        // UI elements
        bool showSettingsTool = false;
        bool drawHelp = true;
    };

    WindowUpdater(hephaestus::VulkanWindowRenderer& _renderer) :
        m_renderer(_renderer)
    {}
    //~WindowUpdater() { Clear(); }
    //void Clear();

    void InitCameraPositionFromMesh(const hephaestus::MeshUtils::TriMesh& mesh) 
    {
        // compute look at camera from the bbox of the data
        hephaestus::AxisAlignedBoundingBox bbox = hephaestus::MeshUtils::ComputeBoundingBox(mesh);
        hephaestus::Vector3 center = hephaestus::Vector3::ZERO;// bbox.ComputeCenter();
        hephaestus::Vector3 camPos = bbox.max;
        camPos.y = center.y;	// move to about the mid axis of the face
        camPos.Mul(3.f);		// move a bit further away

        m_camera.SetLookAt(camPos, center);
        m_renderer.UpdateCamera(m_camera);
    }

    void
    DrawUISettings()
    {
        if (!m_inputUI.showSettingsTool)
        {
            const float DISTANCE = 2.0f;
            // place top left
            ImVec2 window_pos = ImVec2(DISTANCE, DISTANCE);
            ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
            if (ImGui::Begin("Help", &m_inputUI.drawHelp,
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav))
            {
                ImGui::Text("Press F1 for settings");
            }
            ImGui::End();
        }

        if (m_inputUI.showSettingsTool)
        {
            if (ImGui::Begin("Settings", &m_inputUI.showSettingsTool,
                ImGuiWindowFlags_NoNav))//|
                //ImGuiWindowFlags_AlwaysAutoResize))
            {
                if (ImGui::CollapsingHeader("Mesh"))
                {
                    // TODO
                }
                if (ImGui::CollapsingHeader("Camera"))
                {
                    if (ImGui::Button("Reset Camera"))
                        m_inputUI.cameraNeedsReset = true;
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Reset the camera based on the rendered mesh(es)");

                    ImGui::InputFloat("Move Speed", &m_inputUI.cameraMoveStep);
                    ImGui::InputFloat("Rotate Speed", &m_inputUI.cameraRotateStep);

                    ImGui::Text("Mode"); ImGui::SameLine();
                    if (ImGui::RadioButton("LootAt origin",
                        m_inputUI.cameraMode == UIState::CameraMode::eCAMERA_LOOKAT))
                        m_inputUI.cameraMode = UIState::CameraMode::eCAMERA_LOOKAT;
                    ImGui::SameLine();
                    if (ImGui::RadioButton("Free roam",
                        m_inputUI.cameraMode == UIState::CameraMode::eCAMERA_FREE))
                        m_inputUI.cameraMode = UIState::CameraMode::eCAMERA_FREE;
                }
            }
            ImGui::End();
        }
    }


    // Updater
    virtual bool IsReadyToDraw() override { return m_renderer.IsReadyToDraw(); }
    virtual bool OnWindowSizeChanged() override { return m_renderer.OnWindowSizeChanged(); }
    virtual void EndDraw() override {}

    virtual void Update(float /*dtMsecs*/, hephaestus::SimpleWindow::MetricVector& /*metrics*/) override 
    {
        DrawUISettings();
    }

    virtual bool Draw(float dtMsecs, hephaestus::SimpleWindow::MetricVector& metrics) override 
    {
        using namespace hephaestus;

        SwapChainRenderer::RenderStats stats;
        if (!m_renderer.Draw(dtMsecs, stats))
            return false;

        metrics.push_back(SimpleWindow::Metric("Draw Wait", stats.waitTime, SimpleWindow::Metric::eMilliSecs));
        metrics.push_back(SimpleWindow::Metric("Draw Commands", stats.commandTime, SimpleWindow::Metric::eMilliSecs));
        metrics.push_back(SimpleWindow::Metric("Draw Queue", stats.queueTime, SimpleWindow::Metric::eMilliSecs));
        metrics.push_back(SimpleWindow::Metric("Draw Present", stats.presentTime, SimpleWindow::Metric::eMilliSecs));

        return true;
    }
    
    virtual void OnKeyPressed(hephaestus::SimpleWindow::Updater::KeyCode keyCode, float dtMsecs) override
    {
        using namespace hephaestus;

        if (keyCode == Updater::eKEY_F1)
        {
            m_inputUI.showSettingsTool = !m_inputUI.showSettingsTool;
            return;
        }

        const float speed = m_inputUI.cameraMoveStep * dtMsecs;

        // move the camera
        Camera::MoveDirection direction = Camera::eMOVE_NULL;
        switch (keyCode)
        {
        case SimpleWindow::Updater::KeyCode::eKEY_UP:
            direction = Camera::eMOVE_FORWARD;
            break;
        case SimpleWindow::Updater::KeyCode::eKEY_DOWN:
            direction = Camera::eMOVE_BACKWARDS;
            break;
        case SimpleWindow::Updater::KeyCode::eKEY_LEFT:
            direction = Camera::eMOVE_LEFT;
            break;
        case SimpleWindow::Updater::KeyCode::eKEY_RIGHT:
            direction = Camera::eMOVE_RIGHT;
            break;
        default:
            break;
        }

        if (direction != Camera::eMOVE_NULL)
        {
            if (m_inputUI.cameraMode == UIState::CameraMode::eCAMERA_LOOKAT)
                m_camera.MoveLookAt(direction, speed, Vector3::ZERO);   // TODO: look at center of mesh
            else if (m_inputUI.cameraMode)
                m_camera.MoveFreeForm(direction, speed);

            m_renderer.UpdateCamera(m_camera);
        }
    }

    virtual void OnMouseMoved(hephaestus::SimpleWindow::Updater::MouseMovedParams params, float dtMsecs) override
    {
        using namespace hephaestus;
      
        const float speed = m_inputUI.cameraRotateStep * dtMsecs;

        if (m_inputUI.cameraMode == UIState::CameraMode::eCAMERA_FREE)
            m_camera.RotateFromMouseDeltaFreeForm(params.dx, params.dy, speed);
        else if (m_inputUI.cameraMode == UIState::CameraMode::eCAMERA_LOOKAT)
            m_camera.RotateFromMouseDeltaLookAt(
                params.dx, params.dy, speed, Vector3::ZERO);

        m_renderer.UpdateCamera(m_camera);
    }

private:
    // rendering state
    hephaestus::Camera m_camera;
    hephaestus::VulkanWindowRenderer& m_renderer;

    // UI state
    UIState m_inputUI;
};

static void glfw_error_callback(int error, const char* description)
{
    HEPHAESTUS_LOG_ERROR("Glfw Error %d: %s\n", error, description);
}

// Vulkan lib helpers
hephaestus::VulkanDispatcher::ModuleType vulkanLib;
static void UnloadVulkanLib()
{
#ifdef HEPHAESTUS_PLATFORM_WIN32
    if (vulkanLib)
        FreeLibrary(vulkanLib);
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
    if (vulkanLib)
        dlclose(vulkanLib);
#endif
}

// helper for error checking
#define CHECK_EXIT_MSG(expr, msg)	\
if (!(expr)) { HEPHAESTUS_LOG_ERROR(msg); std::exit(EXIT_FAILURE); }



int main(/*int argc, char** argv*/)
{
    constexpr bool enableValidationLayers = true;
    //const char* shaderDir = "../data/shaders/";
    const char* objFilename = "../data/teapot.obj";

    glfwSetErrorCallback(glfw_error_callback);
    CHECK_EXIT_MSG(glfwInit(), "Failed to initialize GLFW");
    CHECK_EXIT_MSG(glfwVulkanSupported(), "GLFW: Vulkan Not Supported");

    // Load the Vulkan dynamic lib
#ifdef HEPHAESTUS_PLATFORM_WIN32
    vulkanLib = LoadLibrary("vulkan-1.dll");
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
    vulkanLib = dlopen("libvulkan.so.1", RTLD_NOW);
#endif
    CHECK_EXIT_MSG(vulkanLib != nullptr, "Failed to load the Vulkan shared library");
    std::atexit(UnloadVulkanLib);

    // setup Vulkan & run app
#ifndef VULKAN_HPP_NO_EXCEPTIONS
    try
#endif
    {
        // create window
        hephaestus::SimpleWindow window;
        hephaestus::SimpleWindow::WindowCreateInfo createInfo;
        createInfo.title = "Vulkan previewer";
        createInfo.nWidth = 1024u;
        createInfo.nHeight = 860u;
        createInfo.startX = 20u;
        createInfo.startY = 20u;
        CHECK_EXIT_MSG(window.Create(createInfo), "Failed to create Window");

        // Imgui setup
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        CHECK_EXIT_MSG(ImGui_ImplGlfw_InitForVulkan(window.GetInfo().window, true), 
            "Failed to initialize ImGui with GLFW and Vulkan");

        // Vulkan Setup
        hephaestus::VulkanDispatcher::InitFromLibrary(vulkanLib);
        hephaestus::VulkanDispatcher::LoadGlobalFunctions();

        hephaestus::VulkanDeviceManager deviceManager;
        {
            hephaestus::VulkanDeviceManager::PlatformWindowInfo platformWindowInfo;
#ifdef HEPHAESTUS_PLATFORM_WIN32
            platformWindowInfo.instance = GetModuleHandle(NULL);
            platformWindowInfo.handle = glfwGetWin32Window(window.GetInfo().window);
#elif defined(HEPHAESTUS_PLATFORM_LINUX)
            platformWindowInfo.display = glfwGetX11Display();
            platformWindowInfo.handle = glfwGetX11Window(window.GetInfo().window);
#else
            static_assert(false, "Unsupported platform");
#endif

            CHECK_EXIT_MSG(deviceManager.Init(platformWindowInfo, enableValidationLayers),
                "Failed to init Device Manager");
        }

        hephaestus::VulkanWindowRenderer renderer(deviceManager);
        CHECK_EXIT_MSG(renderer.Init(), "Failed to initiallize the Vulkan Renderer");

        WindowUpdater updater(renderer);

        // setup shader DB
        hephaestus::VulkanUtils::ShaderDB shaderDB;
        {
            shaderDB.loadedShaders[ShaderType::eSHADER_VERTEX_PNTC] =
                hephaestus::VulkanUtils::CreateShaderModule(deviceManager, "../data/shaders/mesh/mesh.vert.spv");
            shaderDB.loadedShaders[ShaderType::eSHADER_FRAGMENT_PhongTexture] =
                hephaestus::VulkanUtils::CreateShaderModule(deviceManager, "../data/shaders/mesh/mesh.frag.spv");
            shaderDB.loadedShaders[ShaderType::eSHADER_VERTEX_Lines] =
                hephaestus::VulkanUtils::CreateShaderModule(deviceManager, "../data/shaders/primitives/lines.vert.spv");
            shaderDB.loadedShaders[ShaderType::eSHADER_FRAGMENT_Lines] =
                hephaestus::VulkanUtils::CreateShaderModule(deviceManager, "../data/shaders/primitives/lines.frag.spv");
        }

        // preview mesh
        hephaestus::MeshUtils::TriMesh mesh;
        {
            hephaestus::CommonUtils::OBJFileInfo objInfo;
            CHECK_EXIT_MSG(hephaestus::CommonUtils::LoadObjFile(objFilename, mesh, objInfo), "Failed to load file");
            if (!objInfo.hasNormals)
                hephaestus::MeshUtils::ComputeSmoothNormals(mesh);

            // TODO: use loaded texture
            // dummy texture
            std::vector<char> textureData(16, '\xfa');
            hephaestus::MeshUtils::ImageDesc textureDesc = { 2, 2, 4 };
            hephaestus::PipelineBase::ShaderParams shaderParams(shaderDB);
            {
                shaderParams.vertexShaderIndex = ShaderType::eSHADER_VERTEX_PNTC;
                shaderParams.fragmentShaderIndex = ShaderType::eSHADER_FRAGMENT_PhongTexture;
            }
            hephaestus::TriMeshPipeline::SetupParams params;
            params.enableFaceCulling = false;
            CHECK_EXIT_MSG(hephaestus::MeshUtils::SetupPipelineForMesh(
                mesh, textureData, textureDesc, renderer.GetCmdBuffer(), renderer.GetRenderPass(), 
                shaderParams, params, renderer.m_graphicsPipeline),
                "Failed to setup pipeline");

            // compute perspective projection
            const float aspectRatio = ((float)createInfo.nWidth) / ((float)createInfo.nHeight);
            const float fov = 60.f;
            hephaestus::Matrix4 projection;
            hephaestus::CommonUtils::GetPerspectiveProjectionMatrixVulkan(aspectRatio, fov, 0.1f, 100.f, projection);

            std::array<float, 16> matrixData;
            projection.GetRaw(matrixData);
            CHECK_EXIT_MSG(renderer.m_graphicsPipeline.UpdateProjectionMatrix(matrixData, renderer.GetCmdBuffer()),
                "Failed to update projection matrix");
        }

        // primitive pipeline setup
        {
            hephaestus::AxisAlignedBoundingBox bbox = hephaestus::MeshUtils::ComputeBoundingBox(mesh);

            // add a red rectangle around the bounding box of the mesh
            hephaestus::Vector3 center = bbox.ComputeCenter();
            constexpr float color[3] = { 1.f, 0.f, 0.f };
            std::vector<hephaestus::PrimitivesPipeline::VertexData> primData =
            {
                { bbox.min.x, bbox.min.y, center.z, color[0], color[1], color[2] },
                { bbox.max.x, bbox.min.y, center.z, color[0], color[1], color[2] },
                { bbox.max.x, bbox.max.y, center.z, color[0], color[1], color[2] },
                { bbox.min.x, bbox.max.y, center.z, color[0], color[1], color[2] },
                { bbox.min.x, bbox.min.y, center.z, color[0], color[1], color[2] },
            };

            hephaestus::PipelineBase::ShaderParams shaderParams(shaderDB);
            {
                shaderParams.vertexShaderIndex = ShaderType::eSHADER_VERTEX_Lines;
                shaderParams.fragmentShaderIndex = ShaderType::eSHADER_FRAGMENT_Lines;
            }

            CHECK_EXIT_MSG(hephaestus::MeshUtils::SetupPrimitivesPipeline(
                primData, renderer.GetCmdBuffer(), renderer.GetRenderPass(), 
                shaderParams, renderer.m_primitivePipeline),
                "Failed to setup primitives pipeline");
        }

        updater.InitCameraPositionFromMesh(mesh);

        window.RunLoop(updater); // main loop

        deviceManager.WaitDevice();
    }
#ifndef VULKAN_HPP_NO_EXCEPTIONS
    catch (vk::SystemError err)
    {
        HEPHAESTUS_LOG_ERROR("vk::SystemError: %s", err.what());
        std::exit(EXIT_FAILURE);
    }
    catch (...)
    {
        HEPHAESTUS_LOG_ERROR("unknown error");
        std::exit(EXIT_FAILURE);
    }
#endif

    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    return EXIT_SUCCESS;
}