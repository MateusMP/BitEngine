#pragma once

#include <vector>

#include "bitengine/Core/System.h"
#include "bitengine/Core/Window.h"
#include "bitengine/Core/Timer.h"

// Basic Systems
#include "bitengine/Core/VideoSystem.h"
#include "bitengine/Core/InputSystem.h"
#include "bitengine/Core/CommandSystem.h"
#include "bitengine/Core/ResourceSystem.h"
#include "bitengine/Core/ECS/EntitySystem.h"
// Entity System Processors
#include "bitengine/Core/ECS/GameLogicProcessor.h"
#include "bitengine/Core/ECS/Transform2DProcessor.h"
#include "bitengine/Core/ECS/Transform3DProcessor.h"
#include "bitengine/Core/ECS/Camera2DProcessor.h"
#include "bitengine/Core/ECS/Camera3DProcessor.h"
#include "bitengine/Core/ECS/RenderableMeshProcessor.h"

#include "bitengine/Core/Messenger.h"
#include "bitengine/Core/GameEngine.h"

#include "bitengine/Core/GeneralTaskManager.h"

namespace BitEngine 
{
	class GameEnginePC : public GameEngine
	{
		public:
			GameEnginePC(ConfigurationLoader* configLoader, ResourceLoader* resourceLoader, Messenger* messenger);
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
				return resourceLoader;
			}
			virtual Messenger* getMessenger() override {
				return messenger;
			}
			virtual System* getSystem(const std::string& name) override;

			virtual TaskManager* getTaskManager() override {
				return &taskManager;
			}
			
		protected:
			void addSystem(System *sys);

		private:
			bool initSystems();
			void shutdownSystems();


			std::map<std::string, System*> systemsMap;
			std::vector<System*> systems;
			std::vector<System*> systemsToShutdown;

			ConfigurationLoader* configurationLoader;
			ResourceLoader* resourceLoader;
			Messenger* messenger;
			EngineConfiguration configuration;
			GeneralTaskManager taskManager;
	};

}
