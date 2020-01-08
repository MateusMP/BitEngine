#include "BitEngine/Core/Logger.h"
#include "BitEngine/Core/GeneralTaskManager.h"

namespace BitEngine {

TaskWorker::TaskWorker(GeneralTaskManager* _manager, Task::Affinity _affinity, u32 id)
    : m_working(true), m_affinity(_affinity), m_manager(_manager), m_threadId(id)
{
}

void TaskWorker::start()
{
    m_thread = std::thread(&TaskWorker::work, this);
}

// Return false if dit nothing
void TaskWorker::process(TaskPtr task)
{
    BE_PROFILE_FUNCTION();
    if (task->isReady())
    {
        //LOG(EngineLog, BE_LOG_VERBOSE) << " processing task " << task;

        task->execute();

        if (task->isFrameRequired()) {
            m_manager->incFinishedFrameRequired();
        }

        if (task->isRepeating())
        {
            if (task->isOncePerFrame())
            {
                m_manager->scheduleToNextFrame(task);
            }
            else
            {
                m_manager->addTask(task);
            }
        }
        // Task done
    }
    else
    {
        m_manager->addTask(task);
    }
}

void TaskWorker::work()
{
    do
    {
        BE_PROFILE_FUNCTION();
        TaskPtr task = nextTask();
        if (task == nullptr)
        {
            std::this_thread::yield();
        }
        else
        {
            process(task);
        }
    } while (m_working);
    LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Thread ended";
}

TaskPtr TaskWorker::nextTask()
{
    BE_PROFILE_FUNCTION();
    TaskPtr newTask;
    if (!m_taskQueue.tryPop(newTask))
    {
        if (m_affinity == Task::Affinity::BACKGROUND) {
            u32 threadTest = m_threadId++;
            while (threadTest != m_threadId) {
                if (threadTest == 0) {
                    ++threadTest;
                }
                TaskWorker* worker = m_manager->getWorker(threadTest);
                if (worker->m_affinity == Task::Affinity::BACKGROUND && worker->m_taskQueue.tryPop(newTask)) {
                    return newTask;
                }
                ++threadTest;
            }
            std::this_thread::yield();
            m_taskQueue.pop(newTask);
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
    if (m_thread.joinable())
    {
        m_thread.join();
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
    const int nThreads = 4;//std::thread::hardware_concurrency() + 1;
    m_totalWorkers = nThreads;

    workers.resize(m_totalWorkers);
    LOG(EngineLog, BE_LOG_INFO) << "Task manager initializing " << nThreads << " threads";

    for (int i = 0; i < nThreads; ++i)
    {
        workers[i] = new TaskWorker(this, Task::Affinity::BACKGROUND, i);
    }

    // Worker 0 is used for main tasks only
    for (int i = 1; i < nThreads; ++i)
    {
        workers[i]->start();
    }
}

void GeneralTaskManager::update()
{
    BE_PROFILE_FUNCTION();
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
        workers[0]->m_taskQueue.push(task);
    }
    else {
        m_pushCycle = ((m_pushCycle + 1) % (m_totalWorkers-1));
        int at = 1 + m_pushCycle;
        BE_ASSERT(at > 0 && at < m_totalWorkers);
        workers[at]->m_taskQueue.push(task);
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
        i = clampToWorkers(i++);
    } while (i != startedAt);

    if (task == nullptr) {
        std::this_thread::yield();
    }
    else {
        workers[0]->process(task);
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
