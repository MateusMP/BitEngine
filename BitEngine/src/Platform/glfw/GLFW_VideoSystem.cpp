
#include "Platform/glfw/GLFW_VideoSystem.h"

#include "bitengine/Core/Graphics/Material.h"
#include "bitengine/Core/Window.h"

namespace BitEngine
{

GLFW_Window::~GLFW_Window()
{
}

void GLFW_Window::drawBegin()
{
	glfwMakeContextCurrent(window);
}

void GLFW_Window::drawEnd()
{
	glfwSwapBuffers(window);
}

bool GLFW_VideoSystem::init()
{
	LOGCLASS(BE_LOG_VERBOSE) << "Video: Init video...";
	if (!(m_driver.getVideoAdapter() & VideoAdapterType::GL_ANY))
	{
		LOGCLASS(BE_LOG_ERROR) << "GLFW Video System only supports OpenGL drivers.";
		return false;
	}

	glewExperimental = GL_TRUE;
	if (!glfwInit())
	{
		LOGCLASS(BE_LOG_ERROR) << "Video: Failed to initialize glfw!";
		return false;
	}

	glfwSetErrorCallback(GLFW_ErrorCallback);

	if (!m_driver.init())
	{
		LOGCLASS(BE_LOG_ERROR) << "Video: Failed to initialize driver!";
		return false;
	}

	LOGCLASS(BE_LOG_VERBOSE) << "Video initialized!";

	return true;
}

void GLFW_VideoSystem::update()
{
	
}

void GLFW_VideoSystem::shutdown()
{
	m_driver.shutdown();

	for (GLFW_Window *window : m_windows)
	{
		glfwDestroyWindow(window->window);
		delete window;
	}
	m_windows.clear();

	glfwTerminate();
}

void GLFW_VideoSystem::closeWindow(Window *window)
{
	glfwDestroyWindow(static_cast<GLFW_Window *>(window)->window);
}

Window *GLFW_VideoSystem::createWindow(const WindowConfiguration &wc)
{
	GLFW_Window *window = createGLFWWindow(wc);

	BE_ASSERT(window != nullptr);

	if (!glewStarted)
	{
		glewStarted = true;
		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
		{
			LOGCLASS(BE_LOG_ERROR) << "Video: Failed to initialize opengl!";
			glfwTerminate();
			return nullptr;
		}

		// Log to error to force output on all build versions
		LOGCLASS(BE_LOG_INFO) << "Vendor: " << glGetString(GL_VENDOR);
		LOGCLASS(BE_LOG_INFO) << "Renderer: " << glGetString(GL_RENDERER);
		LOGCLASS(BE_LOG_INFO) << "Version: " << glGetString(GL_VERSION);
	}

	return window;
}

GLFW_Window *GLFW_VideoSystem::createGLFWWindow(const WindowConfiguration &wndConf)
{
	// TODO: Use configs

	//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, wndConf.m_Resizable);
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	glfwWindowHint(GLFW_DECORATED, GL_TRUE);

	glfwWindowHint(GLFW_RED_BITS, wndConf.m_RedBits);
	glfwWindowHint(GLFW_GREEN_BITS, wndConf.m_GreenBits);
	glfwWindowHint(GLFW_BLUE_BITS, wndConf.m_BlueBits);
	glfwWindowHint(GLFW_ALPHA_BITS, wndConf.m_AlphaBits);
	glfwWindowHint(GLFW_DEPTH_BITS, wndConf.m_DepthBits);
	glfwWindowHint(GLFW_STENCIL_BITS, wndConf.m_StencilBits);

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor *monitor = nullptr;
	if (wndConf.m_FullScreen)
	{
		monitor = glfwGetPrimaryMonitor();
	}

	GLFWwindow *newWindow = glfwCreateWindow(wndConf.m_Width, wndConf.m_Height, wndConf.m_Title.c_str(), monitor, NULL);
	if (!newWindow)
	{
		LOGCLASS(BE_LOG_ERROR) << "Failed to create window!";
		return nullptr;
	}

	GLFW_Window *window = new GLFW_Window(getMessenger(), newWindow);
	m_windows.push_back(window);

	glfwSetWindowUserPointer(newWindow, window);

	glfwSetFramebufferSizeCallback(newWindow, [](GLFWwindow *window, int width, int height) {
		GLFW_Window *glfwWindow = (GLFW_Window *)glfwGetWindowUserPointer(window);
		glfwWindow->getMessenger()->emit<WindowResizedEvent>(WindowResizedEvent{glfwWindow, width, height});
	});

	glfwSetWindowCloseCallback(newWindow, [](GLFWwindow *window) {
		GLFW_Window *glfwWindow = (GLFW_Window *)glfwGetWindowUserPointer(window);
		glfwWindow->getMessenger()->emit(WindowClosedEvent{glfwWindow});
	});

	glfwMakeContextCurrent(newWindow);

	glfwSwapInterval(1);
	glfwShowWindow(newWindow);

	return window;
}

void GLFW_VideoSystem::GLFW_ErrorCallback(int error, const char *description)
{
	LOGCLASS(BE_LOG_ERROR) << "GLFW Error: " << error << ": " << description;
}

} // namespace BitEngine