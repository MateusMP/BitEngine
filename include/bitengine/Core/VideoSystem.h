#pragma once

#include <set>

#include "bitengine/Core/Signal.h"
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
	class VideoSystem : public MessengerEndpoint
	{
		public:
			VideoSystem(Messenger* m) 
				: MessengerEndpoint(m)
			{
				//getConfig("Fullscreen", "false")->setDescription("Use fullscreen mode, true, false ");
			}
			
			virtual ~VideoSystem() {}
						
			virtual bool init() = 0;
			virtual void shutdown() = 0;
			
			virtual u32 getVideoAdapter() = 0;
			virtual VideoDriver* getDriver() = 0;
			
	};

}


