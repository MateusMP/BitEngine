#pragma once

#include "Graphics.h"
#include "System.h"
#include "Window.h"

#include <set>

namespace BitEngine{

	/**
	 * Override this class for creating your own rendering pipeline
	 */
	class VideoSystem : public System
	{
		public:
			VideoSystem();
			~VideoSystem();
		
			/**
			 * Initializes a window and openGL related stuff (Extensions and functions)
			 * Currently using GLFW and GLEW
			 */
			bool Init() override;

			/**
			 * Close the GLFW system
			 */
			void Shutdown() override;

			/**
			 * Generic implementation
			 * Just clears and updates the screen
			 * Override this function for a custom render pipeline!
			 * Some helper functions are defined on protected space below.
			 */
			void Update() override;

			/**
			 * Called when the window is resized
			 */
			virtual void OnWindowResize(Window* window, int width, int height)
			{
				glViewport(0, 0, width, height);
			}

		protected:
			/** Helper function
			 * Swap buffers drawing new screen
			 */
			void UpdateWindow();

			bool CheckWindowClosed();

			void RecreateWindow();

			void RegisterForResizeCallback(VideoSystem* vs, Window* window);

			// GLFW ONLY
			bool CreateGLFWWindow();
			void DestroyGLFWWindow();


		private:
			Window_glfw m_Window;

			static void GlfwFrameResizeCallback(GLFWwindow* window, int width, int height);

			std::string m_name;

			static std::map<Window*, VideoSystem*> resizeCallbackReceivers;

			// GLFW only
			static std::set<Window*> windowsOpen;
	};

}


