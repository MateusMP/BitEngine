#pragma once

//#ifdef _MSC_VER
#define _USE_MATH_DEFINES // for C++
#include <math.h>
//#else
//#include <cmath>
//#endif

namespace BitEngine{

	const float M_PIf = static_cast<float>(M_PI);
	const float DegToRadf = M_PIf / 180.0f;
	const float RadToDegf = 180.0f / M_PIf;

	static inline float DegToRad(float deg){
		return deg * DegToRadf;
	}

	static inline float RadToDeg(float rad){
		return rad * RadToDegf;
	}
}