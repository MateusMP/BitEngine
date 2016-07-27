#pragma once


#include <memory>

namespace BitEngine{

class Task {
    public:
        // Task Message
        struct TaskCompleted {
            TaskCompleted(Task *t) : m_task(t) {}
            std::shared_ptr<Task> m_task;
        };


        enum class TASK_MODE {
                NONE        =    0x0,

                REPEATING   =    0x1 << 0,
                THREADSAFE  =    0x1 << 1,
                FRAME_SYNC  =    0x1 << 2,

                SINGLETHREADED              = NONE,
                SINGLETHREADED_REPEATING    = REPEATING,
                BACKGROUND                  = THREADSAFE,
                BACKGROUND_REPEATING        = THREADSAFE | REPEATING,
                BACKGROUND_SYNC             = THREADSAFE | FRAME_SYNC,
                BACKGROUND_SYNC_REPEATING   = THREADSAFE | REPEATING | FRAME_SYNC,

                ALL = ~0x0
        };


        Task(TASK_MODE flags);
        virtual ~Task();

        virtual void run() = 0;

        TASK_MODE getFlags() const;

    private:
        TASK_MODE m_flags;
};


}

