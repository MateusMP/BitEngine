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
	void TaskWorker::process(TaskPtr task)
	{
		if (manager->taskReadyToRun(task))
		{
			task->run();

			if (task->isFrameRequired()){
				manager->incFinishedFrameRequired();
			}

			if (task->isRepeating())
			{
				if (task->isOncePerFrame())
				{
					manager->scheduleToNextFrame(task);
				}
				else
				{
					manager->addTask(task);
				}
			}

			manager->getEngine()->getMessenger()->delayedDispatch(MsgTaskCompleted(task));
		}
		else
		{
			manager->addTask(task);
		}
	}

	void TaskWorker::work()
	{
		int k = 0;
		do
		{
			TaskPtr task = nextTask();
			if (task == nullptr)
			{
				std::this_thread::yield();
			}
			else
			{
				process(task);
			}
		} while (working);
	}

	TaskPtr TaskWorker::nextTask()
	{
		TaskPtr newTask;
		if (!taskQueue.tryPop(newTask))
		{
			int queueIdx = nextThread++;
			manager->getWorker(queueIdx)->taskQueue.tryPop(newTask); // TODO: avoid self target?
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

	GeneralTaskManager::GeneralTaskManager(GameEngine* ge)
		: TaskManager(ge)
	{
		mainThread = std::this_thread::get_id();
		LOG(EngineLog, BE_LOG_INFO) << "Main thread: " << mainThread;
		mainloop = true;
		requiredTasksFrame = 0;
		finishedRequiredTasks = 0;
	}

	bool GeneralTaskManager::taskReadyToRun(TaskPtr task)
	{
		if (task->getDependency() == nullptr)
			return true;
		return task->getDependency()->hasPendingWork() == false;
	}

	void GeneralTaskManager::init()
	{
		const int nThreads = std::thread::hardware_concurrency() + 1;

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
		while (mainloop)
		{
			getEngine()->getMessenger()->dispatch(MsgFrameStart());

			while (finishedRequiredTasks != requiredTasksFrame)
			{
				executeMain();
			}

			getEngine()->getMessenger()->dispatch(MsgFrameEnd());

			prepareNextFrame();

			Time::Tick();
		}
	}

	void GeneralTaskManager::prepareNextFrame()
	{
		std::vector<TaskPtr > swaped;
		{
			std::lock_guard<std::mutex> lock(nextFrameTasksMutex);
			swaped.swap(scheduledTasks);
			scheduledTasks.clear();
			addTaskMutex.lock();
			finishedRequiredTasks = 0;
			requiredTasksFrame = 0;
			addTaskMutex.unlock();
		}
		for (TaskPtr& task : swaped)
		{
			addTask(task);
		}
	}

	void GeneralTaskManager::shutdown()
	{
		for (TaskWorker* tw : workers)
		{
			tw->wait();
			delete tw;
		}

		workers.clear();
	}

	void GeneralTaskManager::stop()
	{
		mainloop = false;

		for (TaskWorker* tw : workers)
		{
			tw->stop();
		}
	}

	void GeneralTaskManager::addTask(TaskPtr task)
	{
		{ // lock
			std::lock_guard<std::mutex> lock(addTaskMutex);
			
			if (task->isFrameRequired())
			{
				++requiredTasksFrame;
			}
		} // unlock

		if (task->getAffinity() == Task::Affinity::MAIN) {
			workers[0]->taskQueue.push(task);
		} else {
			getWorker(rand())->taskQueue.push(task);
		}
	}

	void GeneralTaskManager::scheduleToNextFrame(TaskPtr task)
	{
		std::lock_guard<std::mutex> lock(nextFrameTasksMutex);
		scheduledTasks.emplace_back(task);
	}

	void GeneralTaskManager::waitTask(TaskPtr& task)
	{
		if (std::this_thread::get_id() != mainThread)
		{
			throw std::domain_error("Only the main thread may wait for a task!");
		}

		int k = 1;
		while (task->hasPendingWork())
		{
			executeWorkersWork(k++);
		}
	}

	void GeneralTaskManager::executeMain()
	{
		TaskPtr task = workers[0]->nextTask();
		if (task != nullptr) {
			workers[0]->process(task);
		} else {
			executeWorkersWork(0);
		}
	}
	
	void GeneralTaskManager::executeWorkersWork(int i)
	{
		TaskPtr task;
		int startedAt = clampToWorkers(i);
		i = clampToWorkers(i);
		do {
			task = workers[i]->nextTask();
			if (task != nullptr) { break; }
			i = clampToWorkers(i);
		} while (i != startedAt);

		if (task == nullptr) {
			std::this_thread::yield();
		}
	}

	TaskWorker* GeneralTaskManager::getWorker(u32 index)
	{
		return workers[clampToWorkers(index)];
	}

	void GeneralTaskManager::incFinishedFrameRequired()
	{
		std::lock_guard<std::mutex> lock(addTaskMutex);
		++finishedRequiredTasks;
	}

	u32 GeneralTaskManager::clampToWorkers(u32 value)
	{
		return value = 1 + (value % (workers.size() - 1));;
	}
}
