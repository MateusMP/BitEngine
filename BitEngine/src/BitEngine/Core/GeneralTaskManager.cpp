#include "bitengine/Core/Logger.h"
#include "bitengine/Core/GeneralTaskManager.h"

namespace BitEngine {

TaskWorker::TaskWorker(GeneralTaskManager* _manager, Task::Affinity _affinity)
    : working(true), affinity(_affinity), manager(_manager)
{
    nextThread = reinterpret_cast<intptr_t>(this) % 17;
}

void TaskWorker::start()
{
    thread = std::thread(&TaskWorker::work, this);
}

// Return false if dit nothing
void TaskWorker::process(TaskPtr task)
{
    if (task->isReady())
    {
        //LOG(EngineLog, BE_LOG_VERBOSE) << " processing task " << task;

        task->execute();

        if (task->isFrameRequired()) {
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
        // Task done
    }
    else
    {
        manager->addTask(task);
    }
}

void TaskWorker::work()
{
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

        /// TODO: avoid self target?
        if (manager->getWorker(queueIdx)->taskQueue.tryPop(newTask))
        {
            //LOG(EngineLog, BE_LOG_VERBOSE) << "poped: " << newTask << " from " << this;
        }
    }
    else
    {
        //LOG(EngineLog, BE_LOG_VERBOSE) << "poped: " << newTask << " from " << this;
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

GeneralTaskManager::GeneralTaskManager()
    : TaskManager(), mainThread(std::this_thread::get_id())
{
    LOG(EngineLog, BE_LOG_INFO) << "Main thread: " << mainThread;
    requiredTasksFrame = 0;
    finishedRequiredTasks = 0;
    init();
}

void GeneralTaskManager::init()
{
    const int nThreads = 2;//std::thread::hardware_concurrency() + 1;

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
    while (finishedRequiredTasks != requiredTasksFrame)
    {
        executeMain();
    }

    prepareNextFrame();
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
        tw->stop();
    }

    for (TaskWorker* tw : workers)
    {
        tw->wait();
        delete tw;
    }

    workers.clear();
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
    }
    else {
        int at = rand();
        getWorker(at)->taskQueue.push(task);
        //LOG(EngineLog, BE_LOG_VERBOSE) << "pushed: " << task << " to " << getWorker(at);
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
    while (!task->isFinished())
    {
        executeWorkersWork(k++);
    }
}

void GeneralTaskManager::executeMain()
{
    TaskPtr task = workers[0]->nextTask();
    if (task != nullptr) {
        workers[0]->process(task);
    }
    else {
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
