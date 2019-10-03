#pragma once

#ifdef _MSC_VER
#define _USE_MATH_DEFINES // for C++
    #include <math.h>
#else
    #include <cmath>
    #ifndef M_PI
    #define M_PI           3.14159265358979323846
    #endif
#endif

#include "bitengine/Common/TypeDefinition.h"
#include <type_traits>

namespace BitEngine
{
	template <typename T, size_t N>
	constexpr size_t CT_array_size(T(&)[N]){
		return N;
	}

	const float M_PIf = static_cast<float>(M_PI);
	const float DegToRadf = M_PIf / 180.0f;
	const float RadToDegf = 180.0f / M_PIf;

	inline float DegToRad(float deg){
		return deg * DegToRadf;
	}

	inline float RadToDeg(float rad){
		return rad * RadToDegf;
	}

	inline float BytesToMB(u32 bytes) {
		return bytes / (1000000.0f);
	}
	inline float BytesToKB(u32 bytes) {
		return bytes / 1000.0f;
	}

	template<typename T>
	T sign(T x)
	{
		static_assert(std::is_signed<T>::value, "Not a signed type!");
		return static_cast<T>( static_cast<int>(x > static_cast<T>(0.0)) - static_cast<int>(x < static_cast<T>(0.0)));
	}

	template<typename T>
	inline T rotl(T c, u16 by) {
		return (c << by) | (c >> (sizeof(T) * 8 - by));
	}
	template<typename T>
	inline T rotr(T c, u16 by) {
		return (c >> by) | (c << (sizeof(T) * 8 - by));
	}
}
