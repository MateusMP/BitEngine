#include "Core/Logger.h"
#include "Core/GeneralTaskManager.h"

namespace BitEngine{
	
	TaskWorker::TaskWorker(GeneralTaskManager* _manager, Task::Affinity _affinity)
		: manager(_manager), working(true), affinity(_affinity)
	{
		nextThread = ((int)this)%17;
	}

	void TaskWorker::start()
	{
		thread = std::thread(&TaskWorker::work, this);
	}

	// Return false if dit nothing
	bool TaskWorker::mainWork(std::shared_ptr<Task> task)
	{
		if (task.get() != nullptr)
		{
			if (task->getDependency() == nullptr
				|| (task->getDependency() != nullptr && task->getDependency()->finished()))
			{
				task->run();
				if (task->finished())
				{
					manager->getMessenger()->delayedDispatch(TaskCompleted(task));
					task.reset(); // Do not schedule it again.
				}
			}

			// Not finished yet? reschedule.
			if (task.get() != nullptr)
			{
				if (task->getFlags() == Task::TASK_MODE::FRAME_SYNC)
				{
					manager->scheduleToNextFrame(task.get());
				}
				else
				{
					manager->addTask(task.get());
				}
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	void TaskWorker::work()
	{
		if (affinity == Task::Affinity::MAIN)
		{
			mainWork(nextTask());
		}
		else
		{
			int k = 0;
			while (working)
			{
				if (!mainWork(nextTask())) 
				{
					std::this_thread::yield();
					if (!mainWork(nextTask()))
					{
						std::this_thread::sleep_for(std::chrono::microseconds(++k));
						if (k > 100) { k = 100; }
					}
					else 
					{ k = 0; }
				}
			}
		}
	}

	std::shared_ptr<Task> TaskWorker::nextTask()
	{
		std::shared_ptr<Task> newTask;
		if (!taskQueue.tryPop(newTask))
		{
			int queueIdx = nextThread++;
			manager->getWorker(queueIdx)->taskQueue.tryPop(newTask);
		}

		return newTask;
	}

	void TaskWorker::wait()
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}

	GeneralTaskManager::GeneralTaskManager(Messaging::Messenger* m)
		: Messaging::MessengerEndpoint(m)
	{
	}

	void GeneralTaskManager::init()
	{
		const int nThreads = std::thread::hardware_concurrency();
		workers.resize(nThreads);
		LOG(EngineLog, BE_LOG_INFO) << "Task manager initializing " << nThreads << " threads";

		workers[0] = new TaskWorker(this, Task::Affinity::MAIN);
		for (int i = 1; i < nThreads; ++i)
		{
			workers[i] = new TaskWorker(this, Task::Affinity::BACKGROUND);
		}

		for (int i = 1; i < nThreads; ++i)
		{
			workers[i]->start();
		}
	}

	void GeneralTaskManager::update()
	{
		workers[0]->work();

		std::vector<std::shared_ptr<Task> > swaped; 
		{
			std::lock_guard<std::mutex> lock(nextFrameTasksMutex);
			swaped.swap(scheduledTasks);
			scheduledTasks.clear();
		}
		for (std::shared_ptr<Task>& task : swaped)
		{
			addTask(task.get());
		}
	}

	void GeneralTaskManager::shutdown()
	{
		for (TaskWorker* tw : workers)
		{
			tw->stop();
		}

		for (TaskWorker* tw : workers)
		{
			tw->wait();
			delete tw;
		}
	}

	void GeneralTaskManager::addTask(Task *task)
	{
		if (task->getDependency().get() != nullptr)
		{
			dependencyTree.emplace(task->getDependency(), task);
		}

		if (task->getAffinity() == Task::Affinity::MAIN) {
			workers[0]->taskQueue.push(task);
		} else {
			getWorker(rand())->taskQueue.push(task);
		}
	}

	void GeneralTaskManager::scheduleToNextFrame(Task *task)
	{
		std::lock_guard<std::mutex> lock(nextFrameTasksMutex);
		scheduledTasks.emplace_back(task);
	}
	
	TaskWorker* GeneralTaskManager::getWorker(u32 index)
	{
		index = 1 + (index % (workers.size() - 1));
		return workers[index];
	}
}
