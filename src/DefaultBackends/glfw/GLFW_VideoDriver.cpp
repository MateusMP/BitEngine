
#include "DefaultBackends/glfw/GLFW_VideoDriver.h"

#include "Core/Graphics/Material.h"
#include "Core/MessageType.h"


using namespace BitEngine;

void GLFW_VideoDriver::GLFW_ErrorCallback(int error, const char* description)
{
	LOGCLASS(BE_LOG_ERROR) << "GLFW Error: " << error << ": " << description;
}

bool GLFW_VideoDriver::init(const VideoConfiguration& config)
{
	LOGCLASS(BE_LOG_VERBOSE) << "Video: Init video...";

	glewExperimental = GL_TRUE;
	if (!glfwInit()) {
		LOGCLASS(BE_LOG_ERROR) << "Video: Failed to initialize glfw!";
		return false;
	}

	glfwSetErrorCallback(GLFW_ErrorCallback);

	LOGCLASS(BE_LOG_VERBOSE) << "Video initialized!";
	return adapter->init();
}

void GLFW_VideoDriver::shutdown()
{
	for (Window_glfw *w : windowsOpen) {
		closeWindow(w);
	}

	glfwTerminate();
}

void GLFW_VideoDriver::closeWindow(BitEngine::Window* window)
{
	Window_glfw* wglfw = static_cast<Window_glfw*>(window);

	// Destroy
	glfwDestroyWindow(wglfw->m_glfwWindow);
	wglfw->m_glfwWindow = nullptr;

	// Remove from maps
	windowsOpen.erase(wglfw);
	resizeCallbackReceivers.erase(wglfw);
}

Window* GLFW_VideoDriver::recreateWindow(Window* window)
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

Window* GLFW_VideoDriver::createWindow(const WindowConfiguration& wc)
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

void GLFW_VideoDriver::updateWindow(BitEngine::Window* window)
{
	Window_glfw* wglfw = static_cast<Window_glfw*>(window);

	glfwSwapBuffers(wglfw->m_glfwWindow);
}

void GLFW_VideoDriver::update()
{
	for (Window_glfw *w : windowsOpen)
	{
		CheckWindowClosed(w);
	}
}

void GLFW_VideoDriver::clearBuffer(BitEngine::RenderBuffer* buffer, BitEngine::BufferClearBitMask mask)
{
	GLbitfield bitfield = 0;
	if (mask & BitEngine::BufferClearBitMask::COLOR)
		bitfield |= GL_COLOR_BUFFER_BIT;
	if (mask & BitEngine::BufferClearBitMask::DEPTH)
		bitfield |= GL_DEPTH_BUFFER_BIT;

	glClear(bitfield);
}

void GLFW_VideoDriver::clearBufferColor(BitEngine::RenderBuffer* buffer, const BitEngine::ColorRGBA& color)
{
	glClearColor(color.r(), color.g(), color.b(), color.a());
}

void GLFW_VideoDriver::setViewPort(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}

void GLFW_VideoDriver::configure(const BitEngine::Material* material)
{
	// BLEND
	if (material->getState(RenderConfig::BLEND) != BlendConfig::BLEND_NONE)
	{
		GL2::enableState(GL2::getGLState(RenderConfig::BLEND), true);
		if (material->getState(RenderConfig::BLEND) == BlendConfig::BLEND_ALL)
		{
			glBlendFunc(GL2::getBlendMode(material->srcColorBlendMode), GL2::getBlendMode(material->dstColorBlendMode));
		}
		else
		{
			glBlendFuncSeparate(GL2::getBlendMode(material->srcColorBlendMode), GL2::getBlendMode(material->dstColorBlendMode),
								GL2::getBlendMode(material->srcAlphaBlendMode), GL2::getBlendMode(material->dstAlphaBlendMode));
		}
		glBlendEquation(GL2::getBlendEquation(material->blendEquation));
	}
	else
	{
		GL2::enableState(GL2::getGLState(RenderConfig::BLEND), false);
	}

	// ALPHA TEST
	GL2::enableState(GL2::getGLState(RenderConfig::ALPHA_TEST), material->getState(RenderConfig::ALPHA_TEST) == 0);

	// FACE CULL
	if (material->getState(RenderConfig::CULL_FACE) != CULL_FACE_NONE)
	{
		GL2::enableState(GL2::getGLState(RenderConfig::CULL_FACE), true);
		switch (material->getState(RenderConfig::CULL_FACE)) 
		{
			case CullFaceConfig::BACK_FACE:
				glCullFace(GL_BACK);
			break;
			case CullFaceConfig::FRONT_FACE:
				glCullFace(GL_FRONT);
			break;
			case CullFaceConfig::FRONT_AND_BACK:
				glCullFace(GL_FRONT_AND_BACK);
			break;
		}
	}
	else
	{
		GL2::enableState(GL2::getGLState(RenderConfig::CULL_FACE), false);
	}

	u8 depthMode = GL2::getGLState(RenderConfig::DEPTH_TEST);
	if (depthMode &  DepthConfig::DEPTH_TEST_DISABLED) {
		GL2::enableState(GL_DEPTH_TEST, false);
	}
	else if (depthMode &  DepthConfig::DEPTH_TEST_ENABLED) {
		GL2::enableState(GL_DEPTH_TEST, true);
	}
	if (depthMode &  DepthConfig::DEPTH_TEST_WRITE_ENABLED) {
		GL_CHECK(glDepthMask(true));
	} else {
		GL_CHECK(glDepthMask(false));
	}
	GL2::enableState(GL2::getGLState(RenderConfig::DEPTH_TEST),  material->getState(RenderConfig::DEPTH_TEST) == 0);

	GL2::enableState(GL2::getGLState(RenderConfig::MULTISAMPLE), material->getState(RenderConfig::MULTISAMPLE) == 0);
	GL2::enableState(GL2::getGLState(RenderConfig::TEXTURE_1D),  material->getState(RenderConfig::TEXTURE_1D) == 0);
	GL2::enableState(GL2::getGLState(RenderConfig::TEXTURE_2D),  material->getState(RenderConfig::TEXTURE_2D) == 0);
	GL2::enableState(GL2::getGLState(RenderConfig::TEXTURE_3D),  material->getState(RenderConfig::TEXTURE_3D) == 0);
	GL2::enableState(GL2::getGLState(RenderConfig::TEXTURE_CUBE), material->getState(RenderConfig::TEXTURE_CUBE) == 0);
}

bool GLFW_VideoDriver::CheckWindowClosed(Window_glfw* window)
{
	if (glfwWindowShouldClose(window->m_glfwWindow)) {
		getEngine()->getMessenger()->dispatch(MsgWindowClosed(window));
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
