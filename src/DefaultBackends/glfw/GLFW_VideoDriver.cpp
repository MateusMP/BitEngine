
#include "DefaultBackends/glfw/GLFW_VideoDriver.h"

#include "Core/MessageType.h"

using namespace BitEngine;

void GLFW_VideoDriver::GLFW_ErrorCallback(int error, const char* description)
{
	LOGCLASS(BE_LOG_ERROR) << "GLFW Error: " << error << ": " << description;
}

bool GLFW_VideoDriver::Init(const VideoConfiguration& config)
{
	LOGCLASS(BE_LOG_VERBOSE) << "Video: Init video...";

	if (!glfwInit()) {
		LOGCLASS(BE_LOG_ERROR) << "Video: Failed to initialize glfw!";
		return false;
	}

	glfwSetErrorCallback(GLFW_ErrorCallback);

	/*LOGCLASS(BE_LOG_VERBOSE) << "Video: Creating window...";
	Window_glfw *w = new Window_glfw();
	if (!CreateGLFWWindow(w)) {
	LOGCLASS(BE_LOG_ERROR) << "Video: Failed to create video!";
	glfwTerminate();
	return false;
	}*/
	LOGCLASS(BE_LOG_VERBOSE) << "Video initialized!";

	/*glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		LOGCLASS(BE_LOG_ERROR) << "Video: Failed to initialize opengl!";
		glfwTerminate();
		return false;
	}*/

	m_renderer->init();

	return true;
}


void GLFW_VideoDriver::CloseWindow(BitEngine::Window* window)
{
	Window_glfw* wglfw = static_cast<Window_glfw*>(window);

	// Destroy
	glfwDestroyWindow(wglfw->m_glfwWindow);
	wglfw->m_glfwWindow = nullptr;

	// Remove from maps
	windowsOpen.erase(wglfw);
	resizeCallbackReceivers.erase(wglfw);
}

Window* GLFW_VideoDriver::RecreateWindow(Window* window)
{
	Window_glfw* wglfw = static_cast<Window_glfw*>(window);

	if (wglfw->m_glfwWindow)
	{
		CloseWindow(window);
	}

	if (!CreateGLFWWindow(wglfw))
	{
		delete window;
		window = nullptr;
	}

	return window;
}

Window* GLFW_VideoDriver::CreateWindow(const WindowConfiguration& wc)
{
	Window_glfw* window = new Window_glfw();

	window->m_Resizable = wc.m_Resizable;
	window->m_FullScreen = wc.m_FullScreen;

	window->m_Width = wc.m_Width;
	window->m_Height = wc.m_Height;

	window->m_RedBits = wc.m_RedBits;
	window->m_GreenBits = wc.m_GreenBits;
	window->m_BlueBits = wc.m_BlueBits;
	window->m_AlphaBits = wc.m_AlphaBits;

	window->m_DepthBits = wc.m_DepthBits;
	window->m_StencilBits = wc.m_StencilBits;

	window->m_Title = wc.m_Title;

	if (!CreateGLFWWindow(window))
	{
		delete window;
		window = nullptr;
	}
	else
	{
		if (!glewStarted)
		{
			glewStarted = true;
			glewExperimental = GL_TRUE;
			if (glewInit() != GLEW_OK) {
				LOGCLASS(BE_LOG_ERROR) << "Video: Failed to initialize opengl!";
				glfwTerminate();
				return nullptr;
			}

			// Log to error to force output on all build versions
			LOGCLASS(BE_LOG_INFO) << "Vendor: " << glGetString(GL_VENDOR);
			LOGCLASS(BE_LOG_INFO) << "Renderer: " << glGetString(GL_RENDERER);
			LOGCLASS(BE_LOG_INFO) << "Version: " << glGetString(GL_VERSION);
		}

		// This usually is one of the first system initialized
		// so we wait until next frame to deliver this message
		// hoping that everyone that is interested in this message
		// is ready to receive it.
		getMessenger()->delayedDispatch(MsgWindowCreated(window));
	}

	return window;
}


bool GLFW_VideoDriver::CheckWindowClosed(Window_glfw* window)
{
	if (glfwWindowShouldClose(window->m_glfwWindow)) {
		getMessenger()->dispatch(MsgWindowClosed(window));
		return true;
	}

	return false;
}

bool GLFW_VideoDriver::CreateGLFWWindow(Window_glfw* window)
{
	// TODO: Use configs

	//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	if (window->m_glfwWindow) {
		LOGCLASS(BE_LOG_ERROR) << "VideoSystem does not support work with more than one window for now!";
		return false;
	}

	glfwWindowHint(GLFW_RESIZABLE, window->m_Resizable);
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	glfwWindowHint(GLFW_DECORATED, GL_TRUE);

	glfwWindowHint(GLFW_RED_BITS, window->m_RedBits);
	glfwWindowHint(GLFW_GREEN_BITS, window->m_GreenBits);
	glfwWindowHint(GLFW_BLUE_BITS, window->m_BlueBits);
	glfwWindowHint(GLFW_ALPHA_BITS, window->m_AlphaBits);
	glfwWindowHint(GLFW_DEPTH_BITS, window->m_DepthBits);
	glfwWindowHint(GLFW_STENCIL_BITS, window->m_StencilBits);

	GLFWmonitor* monitor = nullptr;
	if (window->m_FullScreen)
		monitor = glfwGetPrimaryMonitor();

	window->m_glfwWindow = glfwCreateWindow(window->m_Width, window->m_Height, window->m_Title.c_str(), monitor, NULL);
	if (!window->m_glfwWindow)
	{
		LOGCLASS(BE_LOG_ERROR) << "Failed to create window!";
		return false;
	}

	glfwSetFramebufferSizeCallback(window->m_glfwWindow, GlfwFrameResizeCallback);

	glfwMakeContextCurrent(window->m_glfwWindow);
	m_currentContext = window;

	glfwSwapInterval(1);
	glfwShowWindow(window->m_glfwWindow);

	windowsOpen.insert(window);
	resizeCallbackReceivers.emplace(window, this);

	return true;
}

void GLFW_VideoDriver::GlfwFrameResizeCallback(GLFWwindow* window, int width, int height)
{
	Window_glfw* resizedWindow = nullptr;
	for (Window_glfw* wg : windowsOpen)
	{
		if (wg->m_glfwWindow == window) {
			resizedWindow = wg;
			break;
		}
	}

	if (resizedWindow == nullptr)
	{
		LOGCLASS(BE_LOG_WARNING) << "Unhandled window resize event!";
		return;
	}

	auto it = resizeCallbackReceivers.find(resizedWindow);
	if (it != resizeCallbackReceivers.end()) {
		it->second->OnWindowResize(resizedWindow, width, height);
	}
	else {
		LOGCLASS(BE_LOG_WARNING) << "No handler registered for window resize event!";
	}
}

std::map<GLFW_VideoDriver::Window_glfw*, GLFW_VideoDriver*> GLFW_VideoDriver::resizeCallbackReceivers;
std::set<GLFW_VideoDriver::Window_glfw*> GLFW_VideoDriver::windowsOpen;
