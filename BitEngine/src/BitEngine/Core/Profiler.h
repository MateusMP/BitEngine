#pragma once

#define BE_PROFILING_CHROME

#ifndef BE_LOG_ENABLE_PERFORMANCE
#define BE_PROFILE_SCOPE(name)		\
		BitEngine::Profiling::PrecisionTimer _profiling##__LINE__(name)
#define BE_PROFILE_FUNCTION()				\
		BE_PROFILE_SCOPE(__FUNCSIG__)
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

namespace BitEngine {

namespace Profiling {
// Adapted from https://gist.github.com/TheCherno/31f135eea6ee729ab5f26a6908eb3a5e
    
static bool enable_profiling = true;

    struct ProfileResult
    {
        std::string name;
        uint32_t threadID;
        long long start, end;
    };

    class ChromeProfiler
    {
    public:
        ChromeProfiler()
            : m_profileCount(0)
        {
        }

        void BeginSession(const std::string& name, const std::string& filepath = "profiling.json")
        {
            m_outputStream.open(filepath);
            WriteHeader();
            m_session.name = name;
        }

        void EndSession()
        {
            WriteFooter();
            m_outputStream.close();
            m_profileCount = 0;
        }

        void WriteProfile(const ProfileResult& result)
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
            m_outputStream.flush();
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

        static ChromeProfiler& Get()
        {
            static ChromeProfiler instance;
            return instance;
        }

    private:
        struct ProfilingSession
        {
            std::string name;
        };

        ProfilingSession m_session;
        std::ofstream m_outputStream;
        int m_profileCount;
    };

    static thread_local long long profiling_timer_last_start = 0;
    class PrecisionTimer
    {
    public:
        PrecisionTimer(const char* name)
            : m_name(name), m_stopped(false)
        {
            auto spoint = std::chrono::high_resolution_clock::now();
            m_start = std::chrono::time_point_cast<std::chrono::microseconds>(spoint).time_since_epoch().count();

            // We need to make sure fast profile calls won't get the same start time,
            // otherwise this can break chrome://tracing view
            if (m_start == profiling_timer_last_start) {
                ++m_start;
            }
            profiling_timer_last_start = m_start;
        }

        ~PrecisionTimer()
        {
            if (!m_stopped) {
                stop();
            }
        }

        void stop()
        {
            auto endTimepoint = std::chrono::high_resolution_clock::now();
            long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

            uint32_t threadID = (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());

            if (Profiling::enable_profiling) {
                ChromeProfiler::Get().WriteProfile({ m_name, threadID, m_start, end });
            }

            m_stopped = true;
        }

    private:
        const char* m_name;
        long long m_start;
        bool m_stopped;
    };

    static void SetProfiling(bool enabled) {
        enable_profiling = enabled;
    }

    static void BeginSession(const char* name) {
#ifdef BE_PROFILING_CHROME
        ChromeProfiler::Get().BeginSession(name);
#else
#endif
    }

    static void EndSession() {
#ifdef BE_PROFILING_CHROME
        ChromeProfiler::Get().EndSession();
#else
#endif
    }

}
}
