#pragma once

#include <chrono>

#include "Common/TypeDefinition.h"

namespace BitEngine
{
	class Time
	{
		public:
			// Used internally
			static void Tick();
			static void ResetTicks();
			static u64 getTicks();

			inline static std::chrono::system_clock::time_point currentTime() {
				return std::chrono::system_clock::now();
			}

			inline static std::chrono::high_resolution_clock::time_point currentTimePrecise() {
				return std::chrono::high_resolution_clock::now();
			}

	};

	// High resolution timer
	class Timer
	{
		public:
			Timer()
			{}
			
			void setTime()
			{
				timeSet = Time::currentTimePrecise();
			}

			// in milliseconds (ms)
			template<typename T>
			T timeElapsedMs() const {
				return std::chrono::duration<T, std::milli>(Time::currentTimePrecise() - timeSet).count();
			}

			// in microseconds (us)
			template<typename T>
			T timeElapsedMicro() const {
				return std::chrono::duration<T, std::micro>(Time::currentTimePrecise() - timeSet).count();
			}

			// in seconds
			template<typename T>
			T timeElapsedSec() const {
				return std::chrono::duration<T>(Time::currentTimePrecise() - timeSet).count();
			}


		private:
			std::chrono::high_resolution_clock::time_point timeSet;
	};

	// Timer based on game ticks (Frames)
	class TimerTicks
	{
	public:
		TimerTicks()
		{}

		void setTime(u64 t = 0)
		{
			if (t == 0) {
				timeSet = Time::getTicks();
			} else {
				timeSet = t;
			}
		}

		u64 timeElapsed() const {
			return Time::getTicks() - timeSet;
		}

	private:
		u64 timeSet;
	};


}