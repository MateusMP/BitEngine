#include "Core/Timer.h"

#ifdef _MSC_VER
#include <Windows.h>
#endif

namespace BitEngine
{
	u64 ticks;

	// Used internally
	void Time::Tick() {
		ticks++;
	}

	// Used internally
	void Time::ResetTicks() {
		ticks = 0;
	}

	u64 Time::getTicks() {
		return ticks;
	}
	

/*
	int64 Timer::clockNow()
	{
	#ifdef _MSC_VER
		LARGE_INTEGER clock;
		QueryPerformanceCounter(&clock);
		return clock.QuadPart;
	#else
		return std::chrono::high_resolution_clock::now();
	#endif
	}

	int64 Timer::clockToMicroSeconds(int64 a)
	{
	#ifdef _MSC_VER
		LARGE_INTEGER freq;
		int64 ret;
		QueryPerformanceFrequency(&freq);

		ret = a * 1000000;
		ret /= freq.QuadPart;
		return ret;
	#else
		return std::chrono::duration_cast<std::chrono::microseconds>(a).count()
	#endif
	}
*/

}
