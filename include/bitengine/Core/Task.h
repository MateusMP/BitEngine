#pragma once

#include <memory>

namespace BitEngine{

class Task;

// Task Message
struct TaskCompleted {
	TaskCompleted(std::shared_ptr<Task> t) : m_task(t) {}
	std::shared_ptr<Task> m_task;
};


class Task {
    public:

		enum class TASK_MODE {
			NONE = 0x0,
			FRAME_SYNC = 0x1 << 1,
		};


		enum class Affinity {
			MAIN = 0,
			BACKGROUND = 1,
		};


        Task(TASK_MODE _flags, Affinity affinity) : flags(_flags) {}
        virtual ~Task() {}

		std::shared_ptr<Task> getDependency() const { return dependency; }
		TASK_MODE getFlags() const  { return flags; }
		Affinity getAffinity() const { return affinity; }
		std::shared_ptr<Task> getDependency() { return dependency; }

        virtual void run() = 0;
		virtual bool finished() = 0;

    private:
        TASK_MODE flags;
		Affinity affinity;
		std::shared_ptr<Task> dependency;
};


}

