#pragma once

#include "BitEngine/Core/api.h"
#include "BitEngine/Common/TypeDefinition.h"

namespace BitEngine {

class BaseComponentHolder;

typedef u32 EntityHandle;
typedef u32 ComponentHandle;
typedef u16 ComponentType;
typedef u16* GlobalComponentID;

const u32 BE_NO_COMPONENT_HANDLE = 0;
const u32 BE_NO_COMPONENT_TYPE = ~0;

class BE_API BaseComponent
{
    friend class BaseComponentHolder;

public:
    virtual ~BaseComponent() {}

protected:
    static ComponentType componentTypeCounter;
};

template<typename T>
class BE_API Component : public BaseComponent
{
public:
    static GlobalComponentID getGlobalComponentID()
    {
        static ComponentType type = componentTypeCounter++;
        return &type;
    }

    static ComponentType getComponentType() {
        return *getGlobalComponentID();
    }
};

}
