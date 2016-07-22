#pragma once

#include <string>

#include "Core/GameEngine.h"
#include "Core/EngineConfiguration.h"

namespace BitEngine{

// System are the basic classes for the GameEngine
// Systems will contain all logic
// A system can have multiple properties, which should be defined on the constructor.
// They are loaded (if found) before the Init call
class System : public EnginePiece
{
    public:
        System(GameEngine* engine) : EnginePiece(engine) {}
        virtual ~System(){}

        virtual const char* getName() const = 0;

        virtual bool Init() = 0;
        virtual void Shutdown() = 0;
        virtual void Update() = 0;

		// Helper function to get config for the system.
		ConfigurationItem* getConfig(const std::string& name, const std::string& defaultValue)
		{
			return getEngine()->getConfigurations()->getConfiguration(getName(), name, defaultValue);
		}

};


}
