#pragma once

#include "Core/Logger.h"

#define BE_ASSERT_THROW_ON_FAIL true

#define ASSERT_FAIL(msg) \
	if (BE_ASSERT_THROW_ON_FAIL) { \
		throw AssertFail(msg);		\
	}

#define ASSERT(expr)	\
	if (BE_DEBUG)		\
		if (!(expr) )	\
			ASSERT_FAIL("Expression failed to validate: <"#expr">");

namespace BitEngine
{
	class AssertFail : std::exception
	{
	public:
		AssertFail(const char* msg)
			: std::exception(msg)
		{
		}
	};

	static void onFailure(const std::string& msg) {
		if (BE_ASSERT_THROW_ON_FAIL) {
			throw AssertFail(msg.c_str());
		}
	}

	static void assertTrue(const bool expected) {
		if (BE_DEBUG) {
			if (!expected) {
				LOG(EngineLog, BE_LOG_ERROR) << "ASSERT True failed.";
				onFailure("ASSERT True failed.");
			}
		}
	}

	template<typename T>
	static void assertEqual(const T& expected, const T& value) {
		if (BE_DEBUG) {
			if (expected != value) {
				LOG(EngineLog, BE_LOG_ERROR) << "ASSERT FAIL expected: " << expected << " got: " << value;
				onFailure("Failed");
			}
		}
	}

	template<typename T>
	static void assertNull(const T& value) {
		if (BE_DEBUG) {
			if (nullptr != value) {
				LOG(EngineLog, BE_LOG_ERROR) << "ASSERT FAIL expected value to be nullptr, got " << value;
				onFailure("ASSERT Null failed.");
			}
		}
	}

	template<typename T>
	static void assertNotNull(const T& value) {
		if (BE_DEBUG) {
			if (nullptr == value) {
				LOG(EngineLog, BE_LOG_ERROR) << "ASSERT FAIL expected value to be non nullptr";
				onFailure("ASSERT value is null failed.");
			}
		}
	}
}