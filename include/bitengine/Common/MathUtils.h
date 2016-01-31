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

#include "Common/TypeDefinition.h"

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

	inline float BytesToMB(uint32 bytes) {
		return bytes / (1000000.0f);
	}
	inline float BytesToKB(uint32 bytes) {
		return bytes / 1000.0f;
	}
}
