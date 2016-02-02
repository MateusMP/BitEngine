#pragma once

#include "Common/TypeDefinition.h"

#define COMPONENT_CLASS() \
    static GlobalComponentID getGlobalComponentID();

#define COMPONENT_CLASS_IMPLEMENTATION(className)           \
    GlobalComponentID className::getGlobalComponentID(){    \
        static int x=0;										\
        return &x;											\
    }

namespace BitEngine{

	class ComponentHolder;

	typedef uint32 EntityHandle;
	typedef uint32 ComponentHandle;
	typedef uint32 ComponentType;
	typedef int* GlobalComponentID;

	const uint32 NO_COMPONENT_HANDLE = 0;
	const uint32 NO_COMPONENT_TYPE = ~0;

	class Component
	{
	public:
		virtual ~Component(){}

		EntityHandle getEntity() const {
			return entity;
		}

	private:
		friend class ComponentHolder;
		EntityHandle entity;
	};

}
