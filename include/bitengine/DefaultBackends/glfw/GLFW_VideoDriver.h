#pragma once

#include "GLFW_Headers.h"
#include "Core/VideoSystem.h"

class GLFW_VideoDriver : public BitEngine::VideoDriver
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

		GLFW_VideoDriver(BitEngine::GameEngine* ge, GLAdapter* _adapter)
			: BitEngine::VideoDriver(ge), glewStarted(false), adapter(_adapter), m_currentContext(nullptr)
		{}
		~GLFW_VideoDriver(){
			delete adapter;
		}

		/**
		* Initializes a window and openGL related stuff (Extensions and functions)
		* Currently using GLFW and GLEW
		*/
		bool init(const BitEngine::VideoConfiguration& config) override;

		/**
		* Close the GLFW system
		*/
		void shutdown() override;

		BitEngine::Window* createWindow(const BitEngine::WindowConfiguration& wc) override;

		void closeWindow(BitEngine::Window* window) override;

		BitEngine::Window* recreateWindow(BitEngine::Window* window) override;

		void updateWindow(BitEngine::Window* window) override;

		void update() override;

		void clearBuffer(BitEngine::RenderBuffer* buffer, BitEngine::BufferClearBitMask mask) override;

		void clearBufferColor(BitEngine::RenderBuffer* buffer, const BitEngine::ColorRGBA& color) override;

		void setViewPort(int x, int y, int width, int height) override;

        void configure(const BitEngine::Material* material) override;
        
		u32 getVideoAdapter() override {
			return adapter->getVideoAdapter();
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

			setViewPort(0, 0, width, height);
			// glViewport(0, 0, width, height);

			if (m_currentContext != window) {
				glfwMakeContextCurrent(m_currentContext->m_glfwWindow);
			}
		}

	private:
		bool glewStarted;
		GLAdapter *adapter;
		Window_glfw* m_currentContext;

		bool CreateGLFWWindow(Window_glfw* window);

		//
		static void GLFW_ErrorCallback(int error, const char* description);
		static void GlfwFrameResizeCallback(GLFWwindow* window, int width, int height);

		static std::map<Window_glfw*, GLFW_VideoDriver*> resizeCallbackReceivers;
		static std::set<Window_glfw*> windowsOpen;

		LOG_CLASS(BitEngine::EngineLog.getOutputSink());
};
