#pragma once

#include <thread>
#include <vector>

#include "Core/Task.h"

namespace BitEngine{


	class TaskManager
	{
		public:
			virtual void init() = 0;
			virtual void update() = 0;
			virtual void shutdown() = 0;

			virtual void addTask(Task *task) = 0;
			virtual void scheduleToNextFrame(Task *task) = 0;

		private:
	};



}

