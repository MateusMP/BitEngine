#pragma once


#include <string>

namespace BitEngine{

class System
{
    public:
        System();
        virtual ~System();

        virtual bool Init() = 0;
        virtual void Shutdown() = 0;
        virtual void Update() = 0;

        virtual const std::string& getName() const = 0;

    protected:

};


}
