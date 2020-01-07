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

namespace BitEngine {

class NonCopyable
{
protected:
    NonCopyable() {}
    ~NonCopyable() {}
private:
    NonCopyable(const NonCopyable&) {}
};

class  NonAssignable
{
protected:
    NonAssignable() {}
    ~NonAssignable() {}
private:
    const NonAssignable& operator=(const NonAssignable&) {}
};


template<typename T>
class Lazy {
public:
    template<typename ...Args>
    Lazy(Args&&... args) {
        new (object) T(std::forward<Args>(args)...);
        ready = true;
    }
    Lazy(Lazy&& other) {
        this->ready = other.ready;
        memcpy(this->object, other.object, sizeof(object));
    }
    Lazy() {
        ready = false;
    }
    ~Lazy() {
        destroy();
    }

    T* operator->() {
        return obj();
    }

    Lazy& operator=(Lazy<T>&& other) {
        this->ready = other.ready;
        memcpy(this->object, other.object, sizeof(object));
        other.ready = false;
        return *this;
    }

    T* obj() {
        return (T*)object;
    }

    void initialized() {
        ready = true;
    }

    void destroy() {
        if (ready) {
            obj()->~T();
            ready = false;
        }
    }
    bool isReady() const {
        return ready;
    }
    operator bool() {
        return ready;
    }

private:
    char object[sizeof(T)];
    bool ready = false;
};

}