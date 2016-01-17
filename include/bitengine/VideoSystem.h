#pragma once

#include "Graphics.h"
#include "System.h"
#include "Window.h"
#include "Video/IVideoRenderer.h"

#include <set>

namespace BitEngine {

	class VideoConfiguration
	{
	};

	class IVideoDriver
	{
		public:
			IVideoDriver(IVideoRenderer* renderer)
				: m_renderer(renderer) 
			{}
		
			virtual bool Init(const VideoConfiguration& config) = 0;
			virtual void Shutdown() = 0;

			virtual Window* CreateWindow(const WindowConfiguration& wc) = 0;
			virtual void CloseWindow(Window* window) = 0;
			virtual Window* RecreateWindow(Window* window) = 0;
			virtual void UpdateWindow(Window* window) = 0;
			
			virtual void Update() = 0;

			IVideoRenderer* getRenderer() {
				return m_renderer;
			}

		protected:
			IVideoRenderer* m_renderer;
	};

	/** Default class for Video configuration
	 * Basic video initialization
	 * Uses just one window
	 */
	class VideoSystem : public System
	{
		public:
			VideoSystem(const std::string& name, IVideoDriver* driver) 
				: System(name), m_driver(driver)
			{
				configuration.AddConfiguration("Fullscreen", "false");
			}

			virtual ~VideoSystem()
			{

			}

			IVideoDriver* getDriver() {
				return m_driver;
			}
					
			/**
			 * Initializes a window and it's rendering driver
			 */
			bool Init() override 
			{
				VideoConfiguration config;
				WindowConfiguration windowConfig;

				windowConfig.m_Title = "WINDOW";
				windowConfig.m_Width = 1280;
				windowConfig.m_Height = 720;
				windowConfig.m_Resizable = GL_TRUE;
				windowConfig.m_FullScreen = configuration.getConfig("Fullscreen")->getValueAsBool();

				windowConfig.m_RedBits = 8;
				windowConfig.m_GreenBits = 8;
				windowConfig.m_BlueBits = 8;
				windowConfig.m_AlphaBits = 8;

				windowConfig.m_DepthBits = 8;
				windowConfig.m_StencilBits = 8;

				if (m_driver->Init(config)) 
				{
					m_window = m_driver->CreateWindow(windowConfig);
					if (m_window)
					{

					}

					return m_window != nullptr;
				}

				return false;
			}

			/**
			 * Close the Video System
			 */
			void Shutdown() override 
			{
				m_driver->CloseWindow(m_window);
				m_driver->Shutdown();
			}

			/**
			 * Called once a frame
			 */
			void Update()
			{
				m_driver->Update();
			}

			/**
			 * Redraws the default window
			 */
			void UpdateWindow()
			{
				m_driver->UpdateWindow(m_window);
			}
			
		protected:
			IVideoDriver* m_driver;

			Window* m_window;
	};

}


