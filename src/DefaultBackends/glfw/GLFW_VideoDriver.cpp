
#include "DefaultBackends/glfw/GLFW_VideoDriver.h"

#include "Core/EngineLoggers.h"
#include "Core/MessageChannel.h"
#include "Core/MessageType.h"

using namespace BitEngine;

bool GLFW_VideoDriver::Init(const VideoConfiguration& config)
{
	LOGTO(Verbose) << "Video: Init video..." << endlog;

	if (!glfwInit()) {
		LOGTO(Error) << "Video: Failed to initialize glfw!" << endlog;
		return false;
	}

	/*LOGTO(Verbose) << "Video: Creating window..." << endlog;
	Window_glfw *w = new Window_glfw();
	if (!CreateGLFWWindow(w)) {
	LOGTO(Error) << "Video: Failed to create video!" << endlog;
	glfwTerminate();
	return false;
	}*/
	LOGTO(Verbose) << "Video initialized!" << endlog;

	/*glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		LOGTO(Error) << "Video: Failed to initialize opengl!" << endlog;
		glfwTerminate();
		return false;
	}*/

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
				LOGTO(Error) << "Video: Failed to initialize opengl!" << endlog;
				glfwTerminate();
				return false;
			}

			// Log to error to force output on all build versions
			LOGTO(Info) << "Vendor: " << glGetString(GL_VENDOR) << endlog;
			LOGTO(Info) << "Renderer: " << glGetString(GL_RENDERER) << endlog;
			LOGTO(Info) << "Version: " << glGetString(GL_VERSION) << endlog;
		}

		Channel::Broadcast<WindowCreated>(WindowCreated(window));
	}

	return window;
}


bool GLFW_VideoDriver::CheckWindowClosed(Window_glfw* window)
{
	if (glfwWindowShouldClose(window->m_glfwWindow)) {
		Channel::Broadcast<WindowClosed>(WindowClosed(window));
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
		LOGTO(Error) << "VideoSystem does not support work with more than one window for now!" << endlog;
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
		LOGTO(Error) << "Failed to create window!" << endlog;
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
		LOGTO(Warning) << "Unhandled window resize event!" << endlog;
		return;
	}

	auto it = resizeCallbackReceivers.find(resizedWindow);
	if (it != resizeCallbackReceivers.end()) {
		it->second->OnWindowResize(resizedWindow, width, height);
	}
	else {
		LOGTO(Warning) << "No handler registered for window resize event!" << endlog;
	}
}

std::map<GLFW_VideoDriver::Window_glfw*, GLFW_VideoDriver*> GLFW_VideoDriver::resizeCallbackReceivers;
std::set<GLFW_VideoDriver::Window_glfw*> GLFW_VideoDriver::windowsOpen;