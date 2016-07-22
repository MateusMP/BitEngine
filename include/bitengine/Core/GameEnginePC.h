#pragma once

#include <vector>

#include "Core/System.h"
#include "Core/MessageType.h"
#include "Core/Graphics.h"
#include "Core/Timer.h"

// Basic Systems
#include "Core/VideoSystem.h"
#include "Core/InputSystem.h"
#include "Core/CommandSystem.h"
#include "Core/ResourceSystem.h"
#include "Core/ECS/EntitySystem.h"
// Entity System Processors
#include "Core/ECS/GameLogicProcessor.h"
#include "Core/ECS/Transform2DProcessor.h"
#include "Core/ECS/Transform3DProcessor.h"
#include "Core/ECS/Camera2DProcessor.h"
#include "Core/ECS/Camera3DProcessor.h"
#include "Core/ECS/Sprite2DProcessor.h"
#include "Core/ECS/RenderableMeshProcessor.h"

#include "Core/Messenger.h"
#include "Core/GameEngine.h"

namespace BitEngine 
{
	class GameEnginePC : public GameEngine
	{
		public:
		GameEnginePC(const std::string& configFile, ResourceLoader* loader);
		~GameEnginePC();

		// Will call the @see CreateSystems() method
		// Load all configurations
		// Call init() on all systems
		// Start the main loop and will block until the game is closed. @see StopRunning()
		bool run();

		// Stop the game main loop.
		// Will finish as soon as the current frame ends.
		void stopRunning();

		virtual EngineConfiguration* getConfigurations() override {
			return &configuration;
		}
		virtual ResourceLoader* getResourceLoader() override {
			return loader;
		}
		virtual Messenger* getMessenger() override {
			return &messenger;
		}
		virtual System* getSystem(const std::string& name) override;

		protected:
		void addSystem(System *sys);

		private:
		bool initSystems();
		void shutdownSystems();

		std::vector<System*> systems;
		std::vector<System*> systemsToShutdown;
		bool running;
		int lastSystemInitialized;

		Messenger messenger;
		EngineConfiguration configuration;
		ResourceLoader* loader;
	};

}