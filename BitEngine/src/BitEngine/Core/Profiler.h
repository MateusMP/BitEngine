#pragma once

#include <BitEngine/Core/api.h>

#define BE_PROFILING_CHROME

#if defined(_MSC_VER)
#define BE_FUNC_SIG __FUNCSIG__
#elif defined(__GNUC__) || defined(__MINGW__) || defined(__clang__)
#define BE_FUNC_SIG __PRETTY_FUNCTION__
#endif

#ifndef BE_LOG_ENABLE_PERFORMANCE
#define BE_PROFILE_SCOPE(name) \
    BitEngine::Profiling::PrecisionTimer _profiling##__LINE__(name)
#define BE_PROFILE_FUNCTION() \
    BE_PROFILE_SCOPE(BE_FUNC_SIG)
#else
#define BE_PROFILE_SCOPE(logto, description)
#define BE_PROFILE_FUNCTION(logto)
#endif

#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

#include "BitEngine/Common/ThreadSafeQueue.h"

namespace BitEngine {

namespace Profiling {
    class ChromeProfiler;

    // Adapted from https://gist.github.com/TheCherno/31f135eea6ee729ab5f26a6908eb3a5e
    struct ProfileResult {
        std::string name;
        uint32_t threadID;
        long long start, end;
    };

    class ChromeProfilerWriter {
    public:
        ChromeProfilerWriter(ThreadSafeQueue<ProfileResult>* dataQueue, const std::string& filepath = "profiling.json")
            : queue(dataQueue)
            , m_profileCount(0)
            , active(1)
        {

            m_outputStream.open(filepath);
        }

        void writeProfile(const ProfileResult& result)
        {
            std::string name = result.name;

            auto elapsed = result.end - result.start;

            // Build string before writing to file
            std::ostringstream entry;

            if (m_profileCount++ > 0) {
                entry << ",";
            }
            entry << "{\"cat\":\"function\",";
            entry << "\"dur\":" << elapsed << ',';
            entry << "\"name\":\"" << name << "\",";
            entry << "\"ph\":\"X\",";
            entry << "\"pid\":0,";
            entry << "\"tid\":" << result.threadID << ",";
            entry << "\"ts\":" << result.start;
            entry << "}";

            // Write
            m_outputStream << entry.str();
            // m_outputStream.flush();
        }

        void work()
        {
            ProfileResult data;
            WriteHeader();

            int cacheFlush = 0;
            while (active) {
                while (active && queue->pop(data)) {
                    writeProfile(data);

                    if (++cacheFlush > 20) {
                        m_outputStream.flush();
                        cacheFlush = 0;
                    }
                }
            }

            while (queue->tryPop(data)) {
                writeProfile(data);
            }

            WriteFooter();
            m_outputStream.close();
            m_profileCount = 0;
        }

        void stop()
        {
            active = false;
        }

        void WriteHeader()
        {
            m_outputStream << "{\"otherData\": {},\"traceEvents\":[";
            m_outputStream.flush();
        }

        void WriteFooter()
        {
            m_outputStream << "]}";
            m_outputStream.flush();
        }

    private:
        ThreadSafeQueue<ProfileResult>* queue;
        std::ofstream m_outputStream;
        int m_profileCount;
        volatile bool active = true;
    };

    class ChromeProfiler {
    public:
        ChromeProfiler()
        {
        }

        void writeProfile(const ProfileResult& result)
        {
            queue.push(result);
        }

        void BeginSession(const std::string& name)
        {
            m_session.name = name;
            writer = new ChromeProfilerWriter(&queue);
            thread = std::thread(&ChromeProfilerWriter::work, writer);
        }

        void EndSession()
        {
            writer->stop();
            queue.notify();
            thread.join();
            delete writer;
        }

        bool enable_profiling = true;

    private:
        struct ProfilingSession {
            std::string name;
        };

        ThreadSafeQueue<ProfileResult> queue;
        ProfilingSession m_session;
        std::thread thread;
        ChromeProfilerWriter* writer;
    };

    extern ChromeProfiler& Get();
    extern void SetInstance(ChromeProfiler* obj);

    // static thread_local long long profiling_timer_last_start = 0;
    class PrecisionTimer {
    public:
        PrecisionTimer(const char* name)
            : m_name(name)
            , m_stopped(false)
        {
            auto spoint = std::chrono::high_resolution_clock::now();
            m_start = std::chrono::time_point_cast<std::chrono::microseconds>(spoint).time_since_epoch().count();

            // We need to make sure fast profile calls won't get the same start time,
            // otherwise this can break chrome://tracing view
            // if (m_start == profiling_timer_last_start) {
            //     ++m_start;
            // }
            // profiling_timer_last_start = m_start;
        }

        ~PrecisionTimer()
        {
            if (!m_stopped) {
                stop();
            }
        }

        void stop()
        {
            if (Profiling::Get().enable_profiling) {
                auto endTimepoint = std::chrono::high_resolution_clock::now();
                long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

                uint32_t threadID = (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());

                Profiling::Get().writeProfile({ m_name, threadID, m_start, end });
            }

            m_stopped = true;
        }

    private:
        const char* m_name;
        long long m_start;
        bool m_stopped;
    };

    static void SetProfiling(bool enabled)
    {
        Profiling::Get().enable_profiling = enabled;
    }

    static void BeginSession(const char* name)
    {
#ifdef BE_PROFILING_CHROME
        Profiling::Get().BeginSession(name);
#else
#endif
    }

    static void EndSession()
    {
#ifdef BE_PROFILING_CHROME
        Profiling::Get().EndSession();
#else
#endif
    }
}
}
