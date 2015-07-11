#pragma once


#include <string>

namespace BitEngine{

class System
{
    public:
        System(const std::string& name);
        virtual ~System();

        virtual bool Init() = 0;
        virtual void Shutdown() = 0;
        virtual void Update() = 0;

        const std::string& getName() const;

    protected:
		std::string m_name;

};


}
