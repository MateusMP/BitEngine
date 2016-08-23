#pragma once

#include <thread>
#include <vector>

#include "Core/Task.h"
#include "Core/GameEngine.h"

namespace BitEngine{


	struct MsgFrameStart{
	};
	struct MsgFrameEnd{
	};

	class TaskManager : public EnginePiece
	{
		public:
			TaskManager(GameEngine * ge) : EnginePiece(ge){}
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

