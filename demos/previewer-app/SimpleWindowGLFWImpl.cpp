#include "SimpleWindow.h"

#include "imgui_impl_vulkan.h"

#include <thread>
#include <chrono>

#include <imgui.h>


#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "imgui_impl_glfw.h"


namespace hephaestus
{
void
SimpleWindow::DrawPerfOverlay(float dtMsecs, const MetricVector& metrics) const
{
    static bool perfOverlayOpen = true;
    const float DISTANCE = 2.0f;
    // place top right
    ImVec2 window_pos = ImVec2(ImGui::GetIO().DisplaySize.x - DISTANCE, DISTANCE);
    ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2(1.f, 0.f));
    if (ImGui::Begin("Perf Overlay", &perfOverlayOpen,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav))
    {
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Update Time: %.3f ms", dtMsecs);
        for (const Metric& metric : metrics)
        {
            if (metric.unit == Metric::eMilliSecs)
                ImGui::Text("%s: %.3f ms", metric.desc.c_str(), metric.value);
            if (metric.unit == Metric::eNatural)
                ImGui::Text("%s: %u", metric.desc.c_str(), (uint32_t)metric.value);
        }
    }
    ImGui::End();
}

void 
SimpleWindow::GLFW_ResizeCallback(GLFWwindow* window, int /*w*/, int /*h*/)
{
    GLFW_CallbackUserInfo* info = reinterpret_cast<GLFW_CallbackUserInfo*>(glfwGetWindowUserPointer(window));

    if (info != nullptr)
        info->resize = true;
}

void 
SimpleWindow::GLFW_KeyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
        GLFW_CallbackUserInfo* info = reinterpret_cast<GLFW_CallbackUserInfo*>(glfwGetWindowUserPointer(window));

        // TODO: change to dispatch table
        Updater::KeyCode keyPressed = Updater::KeyCode::eKEY_NONE;
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            switch (key)
            {
            case GLFW_KEY_ESCAPE:
                keyPressed = Updater::KeyCode::eKEY_ESCAPE;
                break;
            case GLFW_KEY_UP:
                keyPressed = Updater::KeyCode::eKEY_UP;
                break;
            case GLFW_KEY_DOWN:
                keyPressed = Updater::KeyCode::eKEY_DOWN;
                break;
            case GLFW_KEY_LEFT:
                keyPressed = Updater::KeyCode::eKEY_LEFT;
                break;
            case GLFW_KEY_RIGHT:
                keyPressed = Updater::KeyCode::eKEY_RIGHT;
                break;
            case GLFW_KEY_F1:
                keyPressed = Updater::KeyCode::eKEY_F1;
                break;

            default:
                break;
            }
        }

        if (info != nullptr && keyPressed != Updater::KeyCode::eKEY_NONE)
            info->updater->OnKeyPressed(keyPressed, info->dtMsecs);
    }
}

void 
SimpleWindow::GLFW_MouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        GLFW_CallbackUserInfo* info = reinterpret_cast<GLFW_CallbackUserInfo*>(glfwGetWindowUserPointer(window));

        if (info != nullptr)
            info->isLMouseButtonPressed = action == GLFW_PRESS;
    }
}

void 
SimpleWindow::GLFW_CursorPositionCallback(GLFWwindow* window, double /*xpos*/, double /*ypos*/)
{
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        GLFW_CallbackUserInfo* info = reinterpret_cast<GLFW_CallbackUserInfo*>(glfwGetWindowUserPointer(window));

        if (info != nullptr && info->isLMouseButtonPressed)
        {
            Updater::MouseMovedParams params;
            params.dx = ImGui::GetIO().MouseDelta.x;
            params.dy = ImGui::GetIO().MouseDelta.y;
            info->updater->OnMouseMoved(params, info->dtMsecs);
        }
    }
}


SimpleWindow::~SimpleWindow()
{
    glfwDestroyWindow(m_windowInfo.window);
}


bool SimpleWindow::Create(const WindowCreateInfo& createInfo)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); 
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    m_windowInfo.window =
        glfwCreateWindow(createInfo.nWidth, createInfo.nHeight, createInfo.title.c_str(), NULL, NULL);
    if (!m_windowInfo.window)
        return false;

    glfwSetFramebufferSizeCallback(m_windowInfo.window, GLFW_ResizeCallback);
    glfwSetKeyCallback(m_windowInfo.window, SimpleWindow::GLFW_KeyCallback);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(m_windowInfo.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    //glfwSetInputMode(m_windowInfo.window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
    glfwSetMouseButtonCallback(m_windowInfo.window, SimpleWindow::GLFW_MouseButtonCallback);
    glfwSetCursorPosCallback(m_windowInfo.window, SimpleWindow::GLFW_CursorPositionCallback);

    glfwSetWindowUserPointer(m_windowInfo.window, &info);

	return true;
}

bool 
SimpleWindow::RunLoop(SimpleWindow::Updater& updater) const 
{
	bool loop = true;
	bool result = true;

	static bool showImGuiDemo = false;
	float drawTotalTime = 0.f;		// todo: should try to render UI on separate dispatch
    MetricVector metrics;

	auto prevframe = std::chrono::high_resolution_clock::now();
	auto lastRenderTime = prevframe;

	while (loop) 
	{
		auto frameStart = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> duration = frameStart - prevframe;
		const float dtMsecs = (float)duration.count() * 1000.f;
		prevframe = frameStart;

        info.dtMsecs = dtMsecs;
        info.updater = &updater;

        loop = !glfwWindowShouldClose(m_windowInfo.window);
        glfwPollEvents();

        if (info.resize)
        {
            info.resize = false;
			if (!updater.OnWindowSizeChanged())
			{
				result = false;
				break;
			}
		}

		// compensate for vsync input lag by not pushing too many frames in the queue
		std::chrono::duration<double> timeSinceLastRender = frameStart - lastRenderTime;
		if (updater.IsReadyToDraw() &&
			((float)timeSinceLastRender.count()) * 1000.f > 14.f) // allow update on about 60 fps
		{
			lastRenderTime = std::chrono::high_resolution_clock::now();

			ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			// todo: this is actually the draw times from the *last* frame, fix it
			metrics.push_back({ "Draw Total", drawTotalTime, Metric::eMilliSecs });

			// call update and keep metrics
			{
                auto timer_updateStart = std::chrono::high_resolution_clock::now();
                updater.Update(dtMsecs, metrics);
                auto timer_updateEnd = std::chrono::high_resolution_clock::now();

                const float updateTotalTime = 
                    std::chrono::duration<float, std::milli>(timer_updateEnd - timer_updateStart).count();
				DrawPerfOverlay(updateTotalTime, metrics);
			}

			metrics.clear();
			if (showImGuiDemo)
				ImGui::ShowDemoWindow(&showImGuiDemo);

			// Finish ImGui rendering just before we render the full frame
			ImGui::Render();

            auto timer_drawStart = std::chrono::high_resolution_clock::now();
            if (!updater.Draw(dtMsecs, metrics))
			{
				result = false;
				break;
			}
            auto timer_drawEnd = std::chrono::high_resolution_clock::now();

            drawTotalTime =
                std::chrono::duration<float, std::milli>(timer_drawEnd - timer_drawStart).count();

			updater.EndDraw();
		}
	}

	return result;
}

bool 
SimpleWindow::OnExitCommand()
{
    glfwSetWindowShouldClose(m_windowInfo.window, GLFW_TRUE);

    return true;
}

} // namespace hephaestus
