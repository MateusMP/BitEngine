#pragma once

#include <exception>

#include "bitengine/Core/Logger.h"

#define BE_ASSERT_THROW_ON_FAIL true

#define ASSERT_FAIL(msg) \
	if (BE_ASSERT_THROW_ON_FAIL) { \
		throw AssertFail(msg);		\
	}

#define BE_ASSERT(expr)	\
	if (BE_DEBUG)		\
		if (!(expr) )	\
			ASSERT_FAIL("Expression failed to validate: <"#expr">");

namespace BitEngine
{
	class AssertFail : std::exception
	{
	public:
		AssertFail(const std::string& msg)
                    : message(msg)
		{
		}
                    
                const char* what() const noexcept override{
                    return message.c_str();
                }

        private:
            std::string message;
	};

	static void onFailure(const std::string& msg) {
		if (BE_ASSERT_THROW_ON_FAIL) {
			throw AssertFail(msg.c_str());
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
