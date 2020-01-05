#pragma once

#include <cstdint>
#include <cstddef>
#include "BitEngine/Core/api.h"

// unsigned
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// signed
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint32_t bool32;

typedef size_t ptrsize;

#define KILOBYTES(x) (x * 1024)
#define MEGABYTES(x) (KILOBYTES(x) * 1024)

class NonCopyable
{
protected:
    NonCopyable() {}
    ~NonCopyable() {}
private:
    NonCopyable(const NonCopyable&){}
};

class  NonAssignable
{
protected:
    NonAssignable() {}
    ~NonAssignable() {}
private:
    const NonAssignable& operator=(const NonAssignable&){}
};