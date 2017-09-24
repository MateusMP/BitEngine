
#include "bitengine/DefaultBackends/glfw/GLFW_VideoSystem.h"

#include "bitengine/Core/Graphics/Material.h"
#include "bitengine/Core/MessageType.h"

using namespace BitEngine;

std::map<GLFW_VideoSystem::Window_glfw*, GLFW_VideoSystem*> resizeCallbackReceivers;
std::set<GLFW_VideoSystem::Window_glfw*> windowsOpen;

bool GLFW_VideoSystem::Init()
{
	LOGCLASS(BE_LOG_VERBOSE) << "Video: Init video...";
	driver = getEngine()->getVideoDriver();
	if (!(driver->getVideoAdapter() & VideoAdapterType::GL_ANY))
	{
		LOGCLASS(BE_LOG_ERROR) << "GLFW Video System only supports OpenGL drivers.";
		return false;
	}

	glewExperimental = GL_TRUE;
	if (!glfwInit()) {
		LOGCLASS(BE_LOG_ERROR) << "Video: Failed to initialize glfw!";
		return false;
	}

	glfwSetErrorCallback(GLFW_ErrorCallback);

	if (!driver->init()) {
		LOGCLASS(BE_LOG_ERROR) << "Video: Failed to initialize driver!";
		return false;
	}

	LOGCLASS(BE_LOG_VERBOSE) << "Video initialized!";

	WindowConfiguration windowConfig;

	windowConfig.m_Title = "WINDOW";
	windowConfig.m_Width = 1280;
	windowConfig.m_Height = 720;
	windowConfig.m_Resizable = true;
	windowConfig.m_FullScreen = getConfig("Fullscreen", "false")->getValueAsBool();

	windowConfig.m_RedBits = 8;
	windowConfig.m_GreenBits = 8;
	windowConfig.m_BlueBits = 8;
	windowConfig.m_AlphaBits = 8;

	windowConfig.m_DepthBits = 8;
	windowConfig.m_StencilBits = 8;

	m_window = static_cast<Window_glfw*>(createWindow(windowConfig));
	return m_window != nullptr;

	return false;
}

void GLFW_VideoSystem::Update()
{
	for (Window_glfw *w : windowsOpen)
	{
		CheckWindowClosed(w);
	}
}

void GLFW_VideoSystem::Shutdown()
{
	driver->shutdown();
	auto copy = windowsOpen;
	for (Window_glfw *w : copy) {
		closeWindow(w);
	}

	glfwTerminate();
}

void GLFW_VideoSystem::closeWindow(BitEngine::Window* window)
{
	Window_glfw* wglfw = static_cast<Window_glfw*>(window);

	// Destroy
	glfwDestroyWindow(wglfw->m_glfwWindow);
	wglfw->m_glfwWindow = nullptr;

	// Remove from maps
	windowsOpen.erase(wglfw);
	resizeCallbackReceivers.erase(wglfw);
}

Window* GLFW_VideoSystem::recreateWindow(Window* window)
{
	Window_glfw* wglfw = static_cast<Window_glfw*>(window);

	if (wglfw->m_glfwWindow)
	{
		closeWindow(window);
	}

	if (!CreateGLFWWindow(wglfw))
	{
		delete window;
		window = nullptr;
	}

	return window;
}

Window* GLFW_VideoSystem::createWindow(const WindowConfiguration& wc)
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
		getEngine()->getMessenger()->delayedDispatch(MsgWindowCreated(window));
	}

	return window;
}

void GLFW_VideoSystem::updateWindow(BitEngine::Window* window)
{
	Window_glfw* wglfw = static_cast<Window_glfw*>(window);
	if (wglfw == nullptr) {
		wglfw = m_window;
	}

	glfwSwapBuffers(wglfw->m_glfwWindow);
}

bool GLFW_VideoSystem::CheckWindowClosed(Window_glfw* window)
{
	if (glfwWindowShouldClose(window->m_glfwWindow)) {
		getEngine()->getMessenger()->dispatch(MsgWindowClosed(window));
		return true;
	}

	return false;
}

bool GLFW_VideoSystem::CreateGLFWWindow(Window_glfw* window)
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


void GLFW_VideoSystem::GLFW_ErrorCallback(int error, const char* description)
{
	LOGCLASS(BE_LOG_ERROR) << "GLFW Error: " << error << ": " << description;
}

void GLFW_VideoSystem::GlfwFrameResizeCallback(GLFWwindow* window, int width, int height)
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

void GLFW_VideoSystem::OnWindowResize(Window_glfw* window, int width, int height)
{
	if (m_currentContext != window) {
		glfwMakeContextCurrent(window->m_glfwWindow);
	}

	driver->setViewPort(0, 0, width, height);

	if (m_currentContext != window) {
		glfwMakeContextCurrent(m_currentContext->m_glfwWindow);
	}
}
