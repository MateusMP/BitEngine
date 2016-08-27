#pragma once

#include "GLFW_Headers.h"
#include "Core/VideoSystem.h"

class GLFW_VideoSystem : public BitEngine::VideoSystem
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

		GLFW_VideoSystem(BitEngine::GameEngine* ge)
			: BitEngine::VideoSystem(ge), glewStarted(false), m_currentContext(nullptr)
		{}
		~GLFW_VideoSystem() {}

		/**
		* Initializes a window and openGL related stuff (Extensions and functions)
		* Currently using GLFW and GLEW
		*/
		bool Init() override;
		void Shutdown() override;
		void Update() override;
		
		BitEngine::VideoDriver* getDriver() override { return driver; }
		u32 getVideoAdapter() override { return driver->getVideoAdapter(); }

		BitEngine::Window* createWindow(const BitEngine::WindowConfiguration& wc) override;
		void closeWindow(BitEngine::Window* window) override;
		BitEngine::Window* recreateWindow(BitEngine::Window* window) override;
		void updateWindow(BitEngine::Window* window) override;		


	protected:
		/** Helper function
		* Swap buffers drawing new screen
		*/
		bool CheckWindowClosed(Window_glfw* window);

		/**
		* Called when the window is resized
		* Set glViewport for the resized window
		*/
		void OnWindowResize(Window_glfw* window, int width, int height);

	private:
		bool glewStarted;
		BitEngine::VideoDriver *driver;
		Window_glfw* m_currentContext;
		Window_glfw* m_window;

		bool CreateGLFWWindow(Window_glfw* window);

		static void GLFW_ErrorCallback(int error, const char* description);
		static void GlfwFrameResizeCallback(GLFWwindow* window, int width, int height);
		
		LOG_CLASS(BitEngine::EngineLog.getOutputSink());
};
