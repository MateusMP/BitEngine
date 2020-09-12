#pragma once

#include <atomic>
#include <deque>
#include <map>
#include <memory>

#include "BitEngine/Common/ThreadSafeQueue.h"
#include "BitEngine/Core/Messenger.h"
#include "BitEngine/Core/TaskManager.h"

namespace BitEngine {

class GeneralTaskManager;

class TaskWorker
{
    friend class GeneralTaskManager;
public:
    TaskWorker(GeneralTaskManager* _manager, Task::Affinity _affinity, u32 id);

    void stop() {
        m_working = false;
        m_taskQueue.clear();
    }

    // Return true if did some work on a task
    void work();

    // Wait thread to finish
    void wait();

private:
    TaskPtr nextTask();
    void start();
    void process(TaskPtr task);

    bool m_working;
    u32 m_threadId;
    Task::Affinity m_affinity;
    GeneralTaskManager *m_manager;

    std::thread m_thread;
    ThreadSafeQueue< TaskPtr > m_taskQueue;
};

class BE_API GeneralTaskManager : public TaskManager
{
public:
    GeneralTaskManager();
    ~GeneralTaskManager() { shutdown(); }

    void init() override;
    void update() override;
    void shutdown() override;


    void addTask(TaskPtr task) override;
    void scheduleToNextFrame(TaskPtr task) override;
    void waitTask(TaskPtr& task) override;

    const std::vector<TaskPtr>& getTasks() const override { return scheduledTasks; }

    void verifyMainThread() const override {
        BE_ASSERT(std::this_thread::get_id() == mainThread);
    }

private:
    friend class TaskWorker;
    TaskWorker* getWorker(u32 index);
    void prepareNextFrame();

    void executeMain();
    void executeWorkersWork(int i);

    void incFinishedFrameRequired();

    u32 clampToWorkers(u32 value);


    std::vector<TaskWorker*> workers;

    u32 requiredTasksFrame;
    u32 m_totalWorkers;
    u32 m_pushCycle;

    std::mutex addTaskMutex;
    std::mutex nextFrameTasksMutex;
    std::vector<TaskPtr> scheduledTasks;

    const std::thread::id mainThread;

    u32 finishedRequiredTasks;
};
}
