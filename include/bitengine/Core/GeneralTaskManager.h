#pragma once

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

			void stop()
			{
				working = false;
			}

			void wait();
			void work();

		private:
			std::shared_ptr<Task> nextTask();
			void start();
			bool mainWork(std::shared_ptr<Task> task);

			bool working;
			int nextThread;
			Task::Affinity affinity;
			GeneralTaskManager *manager;

			std::thread thread;
			ThreadSafeQueue< std::shared_ptr<Task> > taskQueue;
	};

	class GeneralTaskManager : public TaskManager, public Messaging::MessengerEndpoint
	{
		public:
			GeneralTaskManager(Messaging::Messenger* m);

			void init() override;
			void update() override;
			void shutdown() override;

			void addTask(Task *task) override;

			void scheduleToNextFrame(Task *task) override;

		private:

			friend class TaskWorker;
			TaskWorker* getWorker(u32 index);

			std::vector<TaskWorker*> workers;
			std::map<std::shared_ptr<Task>, std::shared_ptr<Task>> dependencyTree;

			std::mutex nextFrameTasksMutex;
			std::vector<std::shared_ptr<Task> > scheduledTasks;
	};
}