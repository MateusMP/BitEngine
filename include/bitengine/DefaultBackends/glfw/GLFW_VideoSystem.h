#pragma once

#include "GLFW_Headers.h"
#include "bitengine/Core/VideoSystem.h"

struct WindowResized {
	GLFWwindow* window;
	int width;
	int height;
};

class GLFW_VideoSystem : public BitEngine::VideoSystem
{
	public:
		GLFW_VideoSystem(BitEngine::Messenger* m)
			: BitEngine::VideoSystem(m), glewStarted(false)
		{
		}
		~GLFW_VideoSystem() {}

		/**
		* Initializes a window and openGL related stuff (Extensions and functions)
		* Currently using GLFW and GLEW
		*/
		bool init() override;
		void shutdown() override;
		void update();
		
		virtual u32 getVideoAdapter() override {
			return driver.getVideoAdapter();
		}
		virtual BitEngine::VideoDriver* getDriver() override {
			return &driver;
		}

		GLFWwindow* createWindow(const BitEngine::WindowConfiguration& wc);
		void closeWindow(GLFWwindow* window);
		void updateWindow(GLFWwindow* window);


	protected:
		/** Helper function
		* Swap buffers drawing new screen
		*/
		bool checkWindowClosed(GLFWwindow* window);

	private:
		bool glewStarted;
		BitEngine::VideoDriver driver;

		GLFWwindow* createGLFWWindow(const BitEngine::WindowConfiguration& wndConf);

		static void GLFW_ErrorCallback(int error, const char* description);
		static void GlfwFrameResizeCallback(GLFWwindow* window, int width, int height);

		LOG_CLASS(GLFW_VideoSystem, BitEngine::EngineLog);
};
