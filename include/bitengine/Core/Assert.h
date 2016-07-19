#pragma once

#include "Core/Logger.h"

namespace BitEngine
{
	template<typename T>
	static bool assertEqual(const T& a, const T& b) {
		if (BE_DEBUG) 
		{
			if (a == b) 
			{
				return true;
			}
			else
			{
				LOG(EngineLog, BE_LOG_ERROR) << "ASSERT FAIL";
				return false;
			}
		}
	}
}