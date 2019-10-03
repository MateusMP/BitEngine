#pragma once

#include <string>

namespace BitEngine{

	class EngineConfiguration;
	class ResourceLoader;
	class Messenger;
	class System;
	class TaskManager;
	class VideoDriver;
    
	class GameEngine
	{
		public:
		virtual ~GameEngine() {}
		virtual EngineConfiguration* getConfigurations() = 0;
		virtual ResourceLoader* getResourceLoader() = 0;
		virtual TaskManager* getTaskManager() = 0;

		virtual Messenger* getMessenger() = 0;

		virtual System* getSystem(const std::string& name) = 0;
	};

	class EnginePiece
	{
		public:
		EnginePiece(GameEngine* engine)
			: gameEngine(engine) 
		{}

		GameEngine* getEngine() {
			return gameEngine;
		}

		private:
			GameEngine* gameEngine;
	};

}
