#pragma once

#include <chrono>

#include "BitEngine/Common/TypeDefinition.h"

namespace BitEngine {
class Time {
public:
    inline static std::chrono::system_clock::time_point currentTime()
    {
        return std::chrono::system_clock::now();
    }

    inline static std::chrono::high_resolution_clock::time_point currentTimePrecise()
    {
        return std::chrono::high_resolution_clock::now();
    }
};

// High resolution timer
class Timer {
public:
    Timer()
    {
        setTime();
    }

    void setTime()
    {
        timeSet = Time::currentTimePrecise();
    }

    // in milliseconds (ms)
    template <typename T>
    T timeElapsedMs() const
    {
        return std::chrono::duration<T, std::milli>(Time::currentTimePrecise() - timeSet).count();
    }

    // in microseconds (us)
    template <typename T>
    T timeElapsedMicro() const
    {
        return std::chrono::duration<T, std::micro>(Time::currentTimePrecise() - timeSet).count();
    }

    // in seconds
    template <typename T>
    T timeElapsed() const
    {
        return std::chrono::duration<T>(Time::currentTimePrecise() - timeSet).count();
    }

private:
    std::chrono::high_resolution_clock::time_point timeSet;
};
}
