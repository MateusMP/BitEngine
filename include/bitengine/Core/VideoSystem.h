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

	class VideoDriver : public EnginePiece
	{
		friend class VideoSystem;

		public:
			VideoDriver(GameEngine* ge)
				: EnginePiece(ge)
			{}
			virtual ~VideoDriver(){}
		
			virtual bool init(const VideoConfiguration& config) = 0;
			virtual void update() = 0;
			virtual void shutdown() = 0;

			virtual Window* createWindow(const WindowConfiguration& wc) = 0;
			virtual void closeWindow(Window* window) = 0;
			virtual Window* recreateWindow(Window* window) = 0;
			virtual void updateWindow(Window* window) = 0;

			virtual u32 getVideoAdapter() = 0;

			/**
			* Clear buffer on defined options
			* \param buffer Buffer reference, if null, applies to screen
			* \param mask Options to clear, see BufferClearBitMask
			*/
			virtual void clearBuffer(IRenderBuffer* buffer, BufferClearBitMask mask) = 0;

			/**
			* If buffer == nullptr, clears the screen
			* otherwise, clears the given RenderBuffer
			**/
			virtual void clearBufferColor(IRenderBuffer* buffer, const ColorRGBA& color) = 0;

			virtual void setViewPort(int x, int y, int width, int height) = 0;
			
			
	};

	/** Default class for Video configuration
	 * Basic video initialization
	 * Uses just one window
	 */
	class VideoSystem : public System
	{
		public:
			VideoSystem(GameEngine* ge, VideoDriver* driver) 
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

			VideoDriver* getDriver() {
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
				m_driver->closeWindow(m_window);
				m_driver->shutdown();
			}

			/**
			 * Called once a frame
			 */
			void Update()
			{
				m_driver->update();
			}

			/**
			 * Redraws the default window
			 */
			void UpdateWindow()
			{
				m_driver->updateWindow(m_window);
			}
			
		protected:
			VideoDriver* m_driver;

			Window* m_window;
	};

}


