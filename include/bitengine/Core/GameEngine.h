#pragma once

#include <string>
#include "Core/Messenger.h"

namespace BitEngine{

	class EngineConfiguration;
	class ResourceLoader;
	class Messenger;
	class System;

	class GameEngine
	{
		public:
		virtual EngineConfiguration* getConfigurations() = 0;
		virtual ResourceLoader* getResourceLoader() = 0;
		virtual Messaging::Messenger* getMessenger() = 0;
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
