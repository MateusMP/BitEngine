#pragma once

#include "Graphics.h"
#include "VideoSystem.h"

class GLFW_VideoDriver : public BitEngine::IVideoDriver
{
	public:
		class Window_glfw : public BitEngine::Window
		{
			public:
				Window_glfw()
					: m_glfwWindow(nullptr)
				{}

				GLFWwindow* m_glfwWindow;
		};

		GLFW_VideoDriver(BitEngine::IVideoRenderer* renderer)
			: BitEngine::IVideoDriver(renderer), glewStarted(false), m_currentContext(nullptr)
		{}

		/**
		* Initializes a window and openGL related stuff (Extensions and functions)
		* Currently using GLFW and GLEW
		*/
		bool Init(const BitEngine::VideoConfiguration& config) override;

		/**
		* Close the GLFW system
		*/
		void Shutdown() override
		{
			for (Window_glfw *w : windowsOpen) {
				CloseWindow(w);
			}
			
			glfwTerminate();
		}

		BitEngine::Window* CreateWindow(const BitEngine::WindowConfiguration& wc) override;

		void CloseWindow(BitEngine::Window* window) override;

		BitEngine::Window* RecreateWindow(BitEngine::Window* window) override;

		void UpdateWindow(BitEngine::Window* window) override
		{
			Window_glfw* wglfw = static_cast<Window_glfw*>(window);

			glfwSwapBuffers(wglfw->m_glfwWindow);
		}

		void Update() override
		{
			for (Window_glfw *w : windowsOpen)
			{
				CheckWindowClosed(w);
			}
		}

	protected:
		/** Helper function
		* Swap buffers drawing new screen
		*/
		bool CheckWindowClosed(Window_glfw* window);

		/**
		* Called when the window is resized
		* Set glViewport for the resized window
		*/
		virtual void OnWindowResize(Window_glfw* window, int width, int height)
		{
			if (m_currentContext != window) {
				glfwMakeContextCurrent(window->m_glfwWindow);
			}

			m_renderer->setViewPort(0, 0, width, height);
			// glViewport(0, 0, width, height);

			if (m_currentContext != window) {
				glfwMakeContextCurrent(m_currentContext->m_glfwWindow);
			}
		}

	private:
		bool CreateGLFWWindow(Window_glfw* window);

		static void GlfwFrameResizeCallback(GLFWwindow* window, int width, int height);

		static std::map<Window_glfw*, GLFW_VideoDriver*> resizeCallbackReceivers;
		static std::set<Window_glfw*> windowsOpen;

		bool glewStarted;
		Window_glfw* m_currentContext;
};