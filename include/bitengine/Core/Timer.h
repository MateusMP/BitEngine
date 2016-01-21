#pragma once

#include "Common/TypeDefinition.h"

#include "Core/Graphics.h"

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
		// Time since video started
		static double getTime(){ // in seconds
			return glfwGetTime();
		}
		static uint64 getTicks(){

		}

	};

	class Timer
	{
	public:
		Timer(){
			timeSet = 0;
		}

		void setTime(double t = -1)
		{
			if (t <= 0)
				timeSet = Time::getTime();
			else
				timeSet = t;
		}

		bool hasPassed(double seconds) const
		{
			return timeElapsed() >= seconds;
		}

		double timeElapsed() const {
			return Time::getTime() - timeSet;
		}

	private:
		double timeSet;
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