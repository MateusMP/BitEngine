#pragma once

#include "Common/TypeDefinition.h"

namespace BitEngine
{
	class Time
	{
		private:
			friend class GameEngine;
			static uint64 ticks;

			static void Tick(){
				ticks++;
			}

			static void ResetTicks(){
				ticks = 0;
			}

		public:

			static uint64 getTicks() {
				return ticks;
			}
	};

	class Timer
	{
		public:
			Timer()
			{
				timeSet = 0;
			}

			void setTime(int64 t = -1)
			{
				if (t <= 0)
					timeSet = clockNow();
				else
					timeSet = t;
			}

			bool hasPassed(int64 seconds) const
			{
				return timeElapsed() >= (seconds * 1000000);
			}

			// int microseconds
			int64 timeElapsed() const {
				return clockToMicroSeconds(clockNow() - timeSet);
			}

			double timeElapsedSeconds() const {
				return clockToMicroSeconds(clockNow() - timeSet) / 1000000.0;
			}

		private:
			int64 timeSet;

			static int64 clockNow();

			static int64 clockToMicroSeconds(int64 a);
	};

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

		bool hasPassed(uint64 seconds) const
		{
			return (timeSet + seconds) >= Time::getTicks();
		}

		uint64 timeElapsed() const {
			return Time::getTicks() - timeSet;
		}

	private:
		uint64 timeSet;
	};


}