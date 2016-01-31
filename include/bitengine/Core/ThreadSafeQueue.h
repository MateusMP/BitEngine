#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace BitEngine {

	template<typename T>
	class ThreadSafeQueue
	{
		public:
		bool tryPop(T& out)
		{
			if (m_mutex.try_lock())
			{
				if (!m_queue.empty())
				{
					out = m_queue.front();
					m_queue.pop();
					m_mutex.unlock();

					return true;
				}
				else
				{
					m_mutex.unlock();
				}
			}
			return false;
		}

		T pop()
		{
			std::unique_lock<std::mutex> lock(m_mutex);

			while (m_queue.empty()) { // avoid spurious wake ups
				m_cond.wait(lock);
			}

			T a = m_queue.front();
			m_queue.pop();

			return a;
		}

		void push(T&& value)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_queue.emplace(std::forward<T>(value));
			m_cond.notify_one();
		}

		template<typename ...Args>
		void push(Args&& ...value)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_queue.emplace(std::forward<Args>(value)...);
			m_cond.notify_one();
		}

		bool empty()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_queue.empty();
		}

		private:
		std::mutex m_mutex;
		std::condition_variable m_cond;
		std::queue<T> m_queue;
	};
}
