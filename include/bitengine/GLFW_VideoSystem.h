#pragma once

#include "Graphics.h"
#include "VideoSystem.h"

#include "EngineLoggers.h"

#include "MessageChannel.h"
#include "MessageType.h"

class Window_glfw : public BitEngine::Window
{
public:
	Window_glfw()
		: m_glfwWindow(nullptr)
	{}

	GLFWwindow* m_glfwWindow;
};

using namespace BitEngine;
class GLFW_VideoSystem : public BitEngine::VideoSystem
{
public:
	GLFW_VideoSystem()
		: VideoSystem("GLFW_VideoSystem")
	{
		configuration.AddConfiguration("Fullscreen", "0");
		m_Window.m_Title = "WINDOW";
		m_Window.m_Width = 1280;
		m_Window.m_Height = 720;
		m_Window.m_Resizable = GL_TRUE;

		m_Window.m_RedBits = 8;
		m_Window.m_GreenBits = 8;
		m_Window.m_BlueBits = 8;
		m_Window.m_AlphaBits = 8;

		m_Window.m_DepthBits = 8;
		m_Window.m_StencilBits = 8;

	}
	~GLFW_VideoSystem(){}

	/**
	* Initializes a window and openGL related stuff (Extensions and functions)
	* Currently using GLFW and GLEW
	*/
	bool Init() override
	{
		LOGTO(Verbose) << "Video: Init video..." << endlog;

		if (!glfwInit()) {
			LOGTO(Error) << "Video: Failed to initialize glfw!" << endlog;
			return false;
		}

		LOGTO(Verbose) << "Video: Creating window..." << endlog;
		if (!CreateGLFWWindow()) {
			LOGTO(Error) << "Video: Failed to initialize video!" << endlog;
			glfwTerminate();
			return false;
		}
		LOGTO(Verbose) << "Video initialized!" << endlog;

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

		Channel::Broadcast<WindowCreated>(WindowCreated(&m_Window));

		return true;
	}

	/**
	* Close the GLFW system
	*/
	void Shutdown() override
	{
		if (m_Window.m_glfwWindow)
			DestroyGLFWWindow();

		glfwTerminate();
	}

	/**
	* Generic implementation
	* Just clears and updates the screen
	* Override this function for a custom render pipeline!
	* Some helper functions are defined on protected space below.
	*/
	void Update() override
	{
		// EXAMPLE VIDEO UPDATE LAYOUT

		// Check if the window was closed, do nothing in this case.
		if (CheckWindowClosed())
			return;

		// Swap buffers / redraws window
		UpdateWindow();

		// Prepare for next frame
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw more things here ~~ 
	}

	/**
	* Called when the window is resized
	*/
	virtual void OnWindowResize(BitEngine::Window* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

protected:
	/** Helper function
	* Swap buffers drawing new screen
	*/
	void UpdateWindow()
	{
		glfwSwapBuffers(m_Window.m_glfwWindow);
	}

	bool CheckWindowClosed()
	{
		if (glfwWindowShouldClose(m_Window.m_glfwWindow)) {
			Channel::Broadcast<WindowClosed>(WindowClosed(&m_Window));
			return true;
		}

		return false;
	}

	void RecreateWindow()
	{
		if (m_Window.m_glfwWindow) {
			DestroyGLFWWindow();
		}

		CreateGLFWWindow();
	}

	bool CreateGLFWWindow()
	{
		// TODO: Use configs

		//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

		if (m_Window.m_glfwWindow) {
			LOGTO(Error) << "VideoSystem does not support work with more than one window for now!" << endlog;
			return false;
		}

		glfwWindowHint(GLFW_RESIZABLE, m_Window.m_Resizable);
		glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
		glfwWindowHint(GLFW_DECORATED, GL_TRUE);

		glfwWindowHint(GLFW_RED_BITS, m_Window.m_RedBits);
		glfwWindowHint(GLFW_GREEN_BITS, m_Window.m_GreenBits);
		glfwWindowHint(GLFW_BLUE_BITS, m_Window.m_BlueBits);
		glfwWindowHint(GLFW_ALPHA_BITS, m_Window.m_AlphaBits);
		glfwWindowHint(GLFW_DEPTH_BITS, m_Window.m_DepthBits);
		glfwWindowHint(GLFW_STENCIL_BITS, m_Window.m_StencilBits);

		m_Window.m_glfwWindow = glfwCreateWindow(m_Window.m_Width, m_Window.m_Height, m_Window.m_Title.c_str(), NULL, NULL);
		if (!m_Window.m_glfwWindow)
		{
			LOGTO(Error) << "Failed to create window!" << endlog;
			return false;
		}

		glfwSetFramebufferSizeCallback(m_Window.m_glfwWindow, GlfwFrameResizeCallback);

		glfwMakeContextCurrent(m_Window.m_glfwWindow);
		glfwSwapInterval(1);
		glfwShowWindow(m_Window.m_glfwWindow);

		windowsOpen.insert(&m_Window);
		RegisterForResizeCallback(this, &m_Window);

		return true;
	}
	
	void DestroyGLFWWindow()
	{
		glfwDestroyWindow(m_Window.m_glfwWindow);
		m_Window.m_glfwWindow = nullptr;
	}

	void RegisterForResizeCallback(VideoSystem* vs, Window* window)
	{
		resizeCallbackReceivers[window] = vs;
	}

private:
	Window_glfw m_Window;
	static std::map<Window*, VideoSystem*> resizeCallbackReceivers;

	static void GlfwFrameResizeCallback(GLFWwindow* window, int width, int height)
	{
		BitEngine::Window* w = nullptr;
		for (auto it = windowsOpen.begin(); it != windowsOpen.end(); ++it) {
			Window_glfw* glfwW = static_cast<Window_glfw*>(*it);
			if (glfwW->m_glfwWindow == window) {
				w = glfwW;
				break;
			}
		}

		if (w == nullptr) {
			LOGTO(Warning) << "Unhandled window resize event!" << endlog;
			return;
		}

		auto it = resizeCallbackReceivers.find(w);
		if (it != resizeCallbackReceivers.end()) {
			it->second->OnWindowResize(w, width, height);
		}
		else {
			LOGTO(Warning) << "No handler registered for window resize event!" << endlog;
		}
	}

	static std::set<BitEngine::Window*> windowsOpen;
};