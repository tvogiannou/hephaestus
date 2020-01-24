#pragma once

#include <string>
#include <vector>


struct GLFWwindow;

namespace hephaestus
{
class SimpleWindow {
public:

    // Container for metrics populated during the update and displayed by the window later
    struct Metric
    {
        enum Unit
        {
            eMilliSecs = 0,	// value is in milliseconds
            eNatural		// value is a natural number, including 0
        };

        Metric(const std::string& _desc, float _value, Unit _unit) :
            desc(_desc),
            value(_value),
            unit(_unit)
        {}

        std::string desc;
        float value = 0.f;
        Unit unit = eMilliSecs;
    };
    typedef std::vector<Metric> MetricVector;

	struct Updater 
	{
		enum KeyCode : int
		{
			eKEY_NONE = 0,

			eKEY_ESCAPE,
			eKEY_UP,
			eKEY_DOWN,
			eKEY_LEFT,
			eKEY_RIGHT,

			eKEY_F1,
		};

		struct MouseMovedParams
		{
			float dx = 0;
			float dy = 0;
		};

		virtual bool OnWindowSizeChanged() = 0;
		virtual bool IsReadyToDraw() = 0;
		virtual bool Draw(float dtMsecs, MetricVector& metrics) = 0;
		virtual void Update(float dtMsecs, MetricVector& metrics) = 0;
		virtual void EndDraw() = 0;
		virtual void OnKeyPressed(KeyCode keyCode, float dtMsecs) = 0;
		virtual void OnMouseMoved(MouseMovedParams params, float dtMsecs) = 0;
		virtual ~Updater() {}
	};

	struct WindowInfo
	{
        GLFWwindow* window = nullptr;
	};

	SimpleWindow() {}
	~SimpleWindow();

	struct WindowCreateInfo
	{
		std::string title;
		uint32_t nWidth = 1600;
		uint32_t nHeight = 1024;
		uint32_t startY = 20;
		uint32_t startX = 20;
	};
	bool Create(const WindowCreateInfo& createInfo);
	bool RunLoop(Updater& updater) const;

	WindowInfo GetInfo() const { return m_windowInfo; }

private:
	WindowInfo  m_windowInfo;

    // GLFW callbacks for manipulating input
    static void GLFW_ResizeCallback(GLFWwindow* window, int w, int h);
    static void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void GLFW_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void GLFW_CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    
    // info stored GLFWwindow and used by GLFW callbacks
    struct GLFW_CallbackUserInfo
    {
        Updater* updater = nullptr;
        bool isLMouseButtonPressed = false;
        bool resize = true;
        float dtMsecs;
    };
    mutable GLFW_CallbackUserInfo info; // mutable as is manipulated by const RunLoop() method

    bool OnExitCommand();   // to forward 'exit' from the command handler to the appropriate implementation
	void DrawPerfOverlay(float dtMsecs, const MetricVector& metrics) const;
};

}