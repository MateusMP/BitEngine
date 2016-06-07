#pragma once

#include <string>

#include "Core/Messenger.h"
#include "Core/SystemConfiguration.h"

namespace BitEngine{

// System are the basic classes for the GameEngine
// Systems will contain all logic
// A system can have multiple properties, which should be defined on the constructor.
// They are loaded (if found) before the Init call
class System : public MessengerEndpoint
{
	friend class GameEngine;
    public:
        System(const std::string& name);
        virtual ~System(){}

		/**
		 * Should be ready with all default configurations 
		 * available even before Init()!
		 */
		SystemConfiguration* getConfiguration() { return &configuration; }

        const std::string& getName() const;

    protected:
        virtual bool Init() = 0;
        virtual void Shutdown() = 0;
        virtual void Update() = 0;

		std::string m_name;
		SystemConfiguration configuration;

};


}
