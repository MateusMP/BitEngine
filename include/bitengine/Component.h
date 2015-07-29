#pragma once

#include "TypeDefinition.h"

namespace BitEngine{


	typedef uint32 EntityHandle;
	typedef uint32 ComponentHandle;
	typedef uint32 ComponentType;

	class Component
	{
	public:
		virtual ~Component(){}

	};

}