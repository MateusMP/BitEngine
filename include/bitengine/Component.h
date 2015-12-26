#pragma once

#include "TypeDefinition.h"

namespace BitEngine{

	class ComponentHolder;

	typedef uint32 EntityHandle;
	typedef uint32 ComponentHandle;
	typedef uint32 ComponentType;

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