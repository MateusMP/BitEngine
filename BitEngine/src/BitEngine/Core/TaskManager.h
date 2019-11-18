#pragma once

#include <thread>
#include <vector>

#include "BitEngine/Core/Task.h"
#include "BitEngine/Core/Messenger.h"

namespace BitEngine{

	struct MsgFrameStart {};
	struct MsgFrameEnd {};

	class TaskManager
	{
		public:
			TaskManager(){}
			virtual ~TaskManager(){}

			virtual void init() = 0;
			virtual void update() = 0;
			virtual void shutdown() = 0;

			virtual void addTask(std::shared_ptr<Task> task) = 0;
			virtual void scheduleToNextFrame(std::shared_ptr<Task> task) = 0;

			virtual void waitTask(std::shared_ptr<Task>& task) = 0;

            virtual const std::vector<TaskPtr>& getTasks() const = 0;

            virtual const void verifyMainThread() const = 0;

		private:
	};



}

