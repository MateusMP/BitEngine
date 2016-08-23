#pragma once

#include <atomic>
#include <deque>
#include <map>
#include <memory>

#include "Common/ThreadSafeQueue.h"
#include "Core/Messenger.h"
#include "Core/TaskManager.h"

namespace BitEngine {

	class GeneralTaskManager;

	class TaskWorker
	{
		friend class GeneralTaskManager;
		public:
			TaskWorker(GeneralTaskManager* _manager, Task::Affinity _affinity);

			void stop() {
				working = false;
			}

			void wait();

			// Return true if did some work on a task
			void work();

		private:
			TaskPtr nextTask();
			void start();
			void process(TaskPtr task);

			bool working;
			int nextThread;
			Task::Affinity affinity;
			GeneralTaskManager *manager;

			std::thread thread;
			ThreadSafeQueue< TaskPtr > taskQueue;
	};

	class GeneralTaskManager : public TaskManager
	{
		public:
			GeneralTaskManager(GameEngine* ge);
			~GeneralTaskManager(){}

			void init() override;
			void update() override;
			void stop() override;
			void shutdown() override;


			void addTask(TaskPtr task) override;
			void scheduleToNextFrame(TaskPtr task) override;
			void waitTask(TaskPtr& task) override;


		private:
			friend class TaskWorker;
			TaskWorker* getWorker(u32 index);
			void prepareNextFrame();
			bool taskReadyToRun(TaskPtr task);

			void executeMain();
			void executeWorkersWork(int i);

			void incFinishedFrameRequired();

			u32 clampToWorkers(u32 value);

			std::vector<TaskWorker*> workers;

			u32 requiredTasksFrame;

			std::mutex addTaskMutex;
			std::mutex nextFrameTasksMutex;
			std::vector<TaskPtr> scheduledTasks;

			std::thread::id mainThread;

			u32 finishedRequiredTasks;

			bool mainloop;
	};
}
