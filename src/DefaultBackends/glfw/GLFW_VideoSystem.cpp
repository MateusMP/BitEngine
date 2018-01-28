
#include "bitengine/DefaultBackends/glfw/GLFW_VideoSystem.h"

#include "bitengine/Core/Graphics/Material.h"
#include "bitengine/Core/Window.h"

using namespace BitEngine;

std::map<GLFWwindow*, GLFW_VideoSystem*> resizeCallbackReceivers;
std::set<GLFWwindow*> windowsOpen;

bool GLFW_VideoSystem::init()
{
	LOGCLASS(BE_LOG_VERBOSE) << "Video: Init video...";
	if (!(driver.getVideoAdapter() & VideoAdapterType::GL_ANY))
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

	if (!driver.init()) {
		LOGCLASS(BE_LOG_ERROR) << "Video: Failed to initialize driver!";
		return false;
	}

	LOGCLASS(BE_LOG_VERBOSE) << "Video initialized!";

	return true;
}

void GLFW_VideoSystem::update()
{
    std::set<GLFWwindow*> tmp = windowsOpen;
	for (GLFWwindow *w : tmp)
	{
		if (checkWindowClosed(w)) {

			getMessenger()->emit(GLFWWindowClosedMsg{ w });

			windowsOpen.erase(w);
			resizeCallbackReceivers.erase(w);
		}
	}
}

void GLFW_VideoSystem::shutdown()
{
	driver.shutdown();

	auto copy = windowsOpen;
	for (GLFWwindow *w : copy) {
		closeWindow(w);
	}

	glfwTerminate();
}

void GLFW_VideoSystem::closeWindow(GLFWwindow* window)
{
	// Destroy
	glfwDestroyWindow(window);

	// Remove from maps
	windowsOpen.erase(window);
	resizeCallbackReceivers.erase(window);
}

GLFWwindow* GLFW_VideoSystem::createWindow(const WindowConfiguration& wc)
{
	GLFWwindow* window = createGLFWWindow(wc);
	BE_ASSERT(window != nullptr);

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

	return window;
}

void GLFW_VideoSystem::updateWindow(GLFWwindow* window)
{
	glfwSwapBuffers(window);
}

bool GLFW_VideoSystem::checkWindowClosed(GLFWwindow* window)
{
	return glfwWindowShouldClose(window);
}

GLFWwindow* GLFW_VideoSystem::createGLFWWindow(const WindowConfiguration& wndConf)
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor* monitor = nullptr;
	if (wndConf.m_FullScreen) {
		monitor = glfwGetPrimaryMonitor();
	}

	GLFWwindow* window = glfwCreateWindow(wndConf.m_Width, wndConf.m_Height, wndConf.m_Title.c_str(), monitor, NULL);
	if (!window)
	{
		LOGCLASS(BE_LOG_ERROR) << "Failed to create window!";
		return nullptr;
	}

	glfwSetFramebufferSizeCallback(window, GlfwFrameResizeCallback);

	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);
	glfwShowWindow(window);

	windowsOpen.insert(window);
	resizeCallbackReceivers.emplace(window, this);

	return window;
}


void GLFW_VideoSystem::GLFW_ErrorCallback(int error, const char* description)
{
	LOGCLASS(BE_LOG_ERROR) << "GLFW Error: " << error << ": " << description;
}

void GLFW_VideoSystem::GlfwFrameResizeCallback(GLFWwindow* window, int width, int height)
{
	if (window == nullptr)
	{
		LOGCLASS(BE_LOG_WARNING) << "Unhandled window resize event!";
		return;
	}

	auto it = resizeCallbackReceivers.find(window);
	if (it != resizeCallbackReceivers.end()) {
		it->second->getMessenger()->emit<WindowResized>(WindowResized{ window, width, height });
	}
	else {
		LOGCLASS(BE_LOG_WARNING) << "No handler registered for window resize event!";
	}
}
