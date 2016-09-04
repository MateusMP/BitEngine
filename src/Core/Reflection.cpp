#include "Core/Reflection.h"

namespace BitEngine {

	u32 Reflection::globalUniqueValue = 0;

	std::map<Reflection::TypeId, Reflection::ReflectionData*>& Reflection::GetReflectedClasses()
	{
		static std::map<Reflection::TypeId, Reflection::ReflectionData*> reflected;
		return reflected;
	}
}