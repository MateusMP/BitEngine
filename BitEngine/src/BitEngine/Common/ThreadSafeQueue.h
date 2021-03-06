#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace BitEngine {

template <typename T>
class ThreadSafeQueue {
public:
    bool tryPop(T& out)
    {
        if (m_mutex.try_lock()) {
            if (!m_queue.empty()) {
                out = std::move(m_queue.front());
                m_queue.pop();
                m_mutex.unlock();

                return true;
            }
            else {
                m_mutex.unlock();
            }
        }
        return false;
    }

    bool pop(T& out)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            m_cond.wait(lock);
        }

        if (!m_queue.empty()) {
            out = std::move(m_queue.front());
            m_queue.pop();
            return true;
        }
        else {
            return false;
        }
    }

    void clear()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        std::queue<T> empty;
        std::swap(m_queue, empty);
        lock.unlock();
        m_cond.notify_all();
    }

    void push(T&& value)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace(std::forward<T>(value));
        lock.unlock();
        m_cond.notify_one();
    }

    template <typename... Args>
    void push(Args&&... value)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace(std::forward<Args>(value)...);
        lock.unlock();
        m_cond.notify_one();
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    void swap(ThreadSafeQueue& queue)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::lock_guard<std::mutex> lock2(queue.m_mutex);
        m_queue.swap(queue.m_queue);
    }

    void notify()
    {
        m_cond.notify_all();
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::queue<T> m_queue;
};
}
