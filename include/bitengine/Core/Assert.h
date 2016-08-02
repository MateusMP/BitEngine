#pragma once

#include "Core/Logger.h"

namespace BitEngine
{
	template<typename T>
	static void assertEqual(const T& expected, const T& value) {
		if (BE_DEBUG) 
		{
			if (expected != value)
			{
				LOG(EngineLog, BE_LOG_ERROR) << "ASSERT FAIL expected: " << expected << " got: " << value;
			}
		}
	}

	template<typename T>
	static void assertNull(const T& value) {
		if (BE_DEBUG)
		{
			if (nullptr != value)
			{
				LOG(EngineLog, BE_LOG_ERROR) << "ASSERT FAIL expected value to be nullptr, got " << value;
			}
		}
	}
}