#pragma once

#include <string>

#include "Core/Messenger.h"
#include "Core/SystemConfiguration.h"

namespace BitEngine{

class System : public MessengerEndpoint
{
    public:
        System(const std::string& name);
        virtual ~System();

		/**
		 * Should be ready with all default configurations 
		 * available even before Init()!
		 */
		SystemConfiguration* getConfiguration() { return &configuration; }

        virtual bool Init() = 0;
        virtual void Shutdown() = 0;
        virtual void Update() = 0;

        const std::string& getName() const;

    protected:
		std::string m_name;
		SystemConfiguration configuration;

};


}
