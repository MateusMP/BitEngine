#pragma once

#include <set>

#include "bitengine/Core/System.h"
#include "bitengine/Core/Window.h"
#include "bitengine/Core/Messenger.h"
#include "bitengine/Core/Graphics/VideoDriver.h"

namespace BitEngine {
    
    class Material;

	class VideoConfiguration
	{
	};

	/** Default class for Video configuration
	 * Basic video initialization
	 * Uses just one window
	 */
	class VideoSystem : public System
	{
		public:
			VideoSystem(GameEngine* ge) 
				: System(ge)
			{
				getConfig("Fullscreen", "false")->setDescription("Use fullscreen mode, true, false ");
			}
			
			virtual ~VideoSystem() {}

			const char* getName() const override {
				return "Video";
			}
			
			bool Init() = 0;
			void Shutdown() = 0;
			void Update() = 0;

			virtual u32 getVideoAdapter() = 0;
			virtual VideoDriver* getDriver() = 0;
			virtual BitEngine::Window* createWindow(const BitEngine::WindowConfiguration& wc) = 0;
			virtual void closeWindow(BitEngine::Window* window) = 0;
			virtual BitEngine::Window* recreateWindow(BitEngine::Window* window) = 0;

			// If nullptr, updates the default window
			virtual void updateWindow(BitEngine::Window* window = nullptr) = 0;
	};

}


