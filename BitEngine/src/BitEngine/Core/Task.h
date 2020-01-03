#pragma once

#include <atomic>
#include <memory>

#include "BitEngine/Core/Assert.h"

namespace BitEngine {

class Task;

typedef std::shared_ptr<Task> TaskPtr;

// Task Message
struct MsgTaskCompleted {
    MsgTaskCompleted(TaskPtr t) : m_task(t) {}
    std::shared_ptr<Task> m_task;
};


class Task {
public:

    enum class TaskMode {
        NONE = 0x0,
        REPEATING = 0x1 << 1, // Task repeats multiple times.
        ONCE_PER_FRAME = 0x1 << 2, // Task runs a single time per frame.
        FRAME_REQUIRED = 0x1 << 3, // Task that need to be run before the frame ends.

        NOT_VALID = REPEATING | FRAME_REQUIRED,

        REPEAT_ONCE_PER_FRAME_REQUIRED = REPEATING | ONCE_PER_FRAME | FRAME_REQUIRED,
        REPEAT_ONCE_PER_FRAME = REPEATING | ONCE_PER_FRAME,
    };

    enum class Affinity {
        MAIN = 0,
        BACKGROUND = 1,
    };


    Task(TaskMode _flags, Affinity _affinity)
        : flags(_flags), affinity(_affinity), remainingWork(1) {}
    virtual ~Task() {}

    TaskMode getFlags() const { return flags; }
    Affinity getAffinity() const { return affinity; }

    void execute() {
        run();
        if (isRepeating()) {
            remainingWork = 1;
        }
        else {
            remainingWork = 0;
        }
    }

    bool isFinished() {
        return remainingWork == 0;
    }

    bool isReady() {
        if (remainingWork > 1)
        {
            int remaining = 0;
            for (TaskPtr& t : waitingTasks) {
                if (!t->isFinished()) {
                    ++remaining;
                }
            }
            remainingWork = 1 + remaining;
        }

        return remainingWork <= 1;
    }

    void addDependency(TaskPtr task) {
        ++remainingWork;
        waitingTasks.emplace_back(task);
    }

    bool isRepeating() {
        return (enum_value(flags) & enum_value(TaskMode::REPEATING)) > 0;
    }

    bool isFrameRequired() {
        return (enum_value(flags) & enum_value(TaskMode::FRAME_REQUIRED)) > 0;
    }

    bool isOncePerFrame() {
        return (enum_value(flags) & enum_value(TaskMode::ONCE_PER_FRAME)) > 0;
    }

    const std::vector<TaskPtr>& getDependencies() const {
        return waitingTasks;
    }

    void stopRepeating() {
        flags = TaskMode(enum_value(flags) & !enum_value(TaskMode::REPEATING));
    }

protected:
    void setAffinity(Affinity a) {
        affinity = a;
    }

private:
    virtual void run() = 0;

    TaskMode flags;
    Affinity affinity;
    std::vector<TaskPtr> waitingTasks; // tasks this task must wait before it can run
    std::atomic<u32> remainingWork;

    template <typename T>
    static constexpr typename std::underlying_type<T>::type enum_value(T val) {
        return static_cast<typename std::underlying_type<T>::type>(val);
    }
};


}

