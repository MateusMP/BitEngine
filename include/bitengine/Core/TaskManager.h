#pragma once

#include <thread>
#include <vector>

#include "bitengine/Core/Task.h"
#include "bitengine/Core/Messenger.h"
#include "bitengine/Core/GameEngine.h"

namespace BitEngine{

	struct MsgFrameStart {};
	struct MsgFrameEnd {};

	class TaskManager : public MessengerEndpoint
	{
		public:
			TaskManager(Messenger* m) : MessengerEndpoint(m){}
			virtual ~TaskManager(){}

			virtual void init() = 0;
			virtual void update() = 0;
			virtual void stop() = 0;
			virtual void shutdown() = 0;

			virtual void addTask(std::shared_ptr<Task> task) = 0;
			virtual void scheduleToNextFrame(std::shared_ptr<Task> task) = 0;

			virtual void waitTask(std::shared_ptr<Task>& task) = 0;

		private:
	};



}

