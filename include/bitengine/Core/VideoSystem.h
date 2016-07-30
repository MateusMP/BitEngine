#pragma once

#include <set>

#include "Core/Graphics.h"
#include "Core/System.h"
#include "Core/Window.h"
#include "Core/VideoRenderer.h"
#include "Core/Messenger.h"
#include "Core/EngineConfiguration.h"

namespace BitEngine {

	class VideoConfiguration
	{
	};

	class IVideoDriver : public Messaging::MessengerEndpoint
	{
		friend class VideoSystem;

		public:
			IVideoDriver(Messenger* m, VideoRenderer* renderer)
				: MessengerEndpoint(m), m_renderer(renderer)
			{}
			virtual ~IVideoDriver(){
				delete m_renderer;
			}
		
			virtual bool Init(const VideoConfiguration& config) = 0;
			virtual void Shutdown() = 0;

			virtual Window* CreateWindow(const WindowConfiguration& wc) = 0;
			virtual void CloseWindow(Window* window) = 0;
			virtual Window* RecreateWindow(Window* window) = 0;
			virtual void UpdateWindow(Window* window) = 0;
			
			virtual void Update() = 0;

			VideoRenderer* getRenderer() {
				return m_renderer;
			}

		protected:
			VideoRenderer* m_renderer;
	};

	/** Default class for Video configuration
	 * Basic video initialization
	 * Uses just one window
	 */
	class VideoSystem : public System
	{
		public:
			VideoSystem(GameEngine* ge, IVideoDriver* driver) 
				: System(ge), m_driver(driver)
			{
				getConfig("Fullscreen", "false")->setDescription("Use fullscreen mode, true, false ");
			}


			virtual ~VideoSystem()
			{
				delete m_driver;
			}

			const char* getName() const override {
				return "Video";
			}

			IVideoDriver* getDriver() {
				return m_driver;
			}

			/**
			 * Initializes a window and it's rendering driver
			 */
			bool Init() override;

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


