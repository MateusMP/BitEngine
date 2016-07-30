#pragma once

#include <atomic>
#include <memory>

namespace BitEngine{

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
			REPEATING			= 0x1 << 1, // Task repeats multiple times.
			ONCE_PER_FRAME		= 0x1 << 2, // Task runs a single time per frame.
			FRAME_REQUIRED		= 0x1 << 3, // Task that need to be run before the frame ends.

			NOT_VALID = REPEATING | FRAME_REQUIRED,

			REPEAT_ONCE_PER_FRAME_REQUIRED = REPEATING | ONCE_PER_FRAME | FRAME_REQUIRED,
			REPEAT_ONCE_PER_FRAME = REPEATING | ONCE_PER_FRAME,
		};

		enum class Affinity {
			MAIN = 0,
			BACKGROUND = 1,
		};


        Task(TaskMode _flags, Affinity _affinity) : flags(_flags), affinity(_affinity) {}
        virtual ~Task() {}

		std::shared_ptr<Task> getDependency() const { return dependency; }
		TaskMode getFlags() const  { return flags; }
		Affinity getAffinity() const { return affinity; }
		std::shared_ptr<Task> getDependency() { return dependency; }

        virtual void run() = 0;

		void addChild(TaskPtr task) {
			++pending_work;
		}
		
		bool hasPendingWork() {
			return pending_work > 0;
		}

		void childFinished() {
			--pending_work;
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

    private:

        TaskMode flags;
		Affinity affinity;
		std::shared_ptr<Task> parent;
		std::shared_ptr<Task> dependency;
		std::atomic<u32> pending_work;

		template <typename T>
		static constexpr typename std::underlying_type<T>::type enum_value(T val) {
			return static_cast<typename std::underlying_type<T>::type>(val);
		}
};


}

