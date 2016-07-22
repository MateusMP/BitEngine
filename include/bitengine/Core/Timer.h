#pragma once

#include <chrono>

#include "Common/TypeDefinition.h"

namespace BitEngine
{
	class Time
	{
		private:
			static uint64 ticks;

		public:
			// Used internally
			static void Tick(){
				ticks++;
			}

			// Used internally
			static void ResetTicks(){
				ticks = 0;
			}

			inline static std::chrono::system_clock::time_point currentTime() {
				return std::chrono::system_clock::now();
			}

			inline static std::chrono::high_resolution_clock::time_point currentTimePrecise() {
				return std::chrono::high_resolution_clock::now();
			}

			static uint64 getTicks() {
				return ticks;
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

			// in milliseconds
			double timeElapsedMs() const {
				return std::chrono::duration<double, std::milli>(Time::currentTimePrecise() - timeSet).count();
			}

			// in microseconds
			double timeElapsedMicro() const {
				return std::chrono::duration<double, std::micro>(Time::currentTimePrecise() - timeSet).count();
			}

			// in seconds
			double timeElapsedSec() const {
				return std::chrono::duration<double>(Time::currentTimePrecise() - timeSet).count();
			}


		private:
			std::chrono::high_resolution_clock::time_point timeSet;
	};

	// Timer based on game ticks (Frames)
	class TimerTicks
	{
	public:
		TimerTicks(){

		}

		void setTime(uint64 t = 0)
		{
			if (t == 0)
				timeSet = Time::getTicks();
			else
				timeSet = t;
		}

		uint64 timeElapsed() const {
			return Time::getTicks() - timeSet;
		}

	private:
		uint64 timeSet;
	};


}