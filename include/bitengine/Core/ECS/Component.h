#pragma once

#include "Common/TypeDefinition.h"

namespace BitEngine{

	class BaseComponentHolder;

	typedef uint32 EntityHandle;
	typedef uint32 ComponentHandle;
	typedef uint16 ComponentType;
	typedef uint16* GlobalComponentID;

	const uint32 BE_NO_COMPONENT_HANDLE = 0;
	const uint32 BE_NO_COMPONENT_TYPE = ~0;

	class BaseComponent
	{
		friend class BaseComponentHolder;

		public:
			virtual ~BaseComponent(){}

		protected:
			static ComponentType componentTypeCounter;
	};

	template<typename T>
	class Component : public BaseComponent
	{
		public:
			static GlobalComponentID getGlobalComponentID()
			{
				static ComponentType type = ++componentTypeCounter;
				return &type;
			}

			static ComponentType getComponentType() {
				return *getGlobalComponentID();
			}
	};

}
