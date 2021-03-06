#pragma once

#include <exception>

#include "BitEngine/Core/api.h"
#include "BitEngine/Core/Logger.h"

#define BE_ASSERT_THROW_ON_FAIL true

#define ASSERT_FAIL(msg)                  \
    if (BE_ASSERT_THROW_ON_FAIL) {        \
        throw BitEngine::AssertFail(msg); \
    }

#ifdef BE_DEBUG
#define BE_ASSERT(expr)                                            \
    if (!(expr)) {                                                 \
        ASSERT_FAIL("Expression failed to validate: <" #expr ">"); \
    }
#else
#define BE_ASSERT(expr)
#endif

#define BE_INVALID_PATH(msg) \
    throw BitEngine::AssertFail(msg)

namespace BitEngine {
class AssertFail : std::exception {
public:
    AssertFail(const std::string& msg)
        : message(msg)
    {
        LOG(BitEngine::EngineLog, BE_LOG_ERROR) << msg;
    }

    const char* what() const noexcept override
    {
        return message.c_str();
    }

private:
    std::string message;
};
}
