#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <thread>

#include "Core/Timer.h"
#include "Common/MacroHelpers.h"

//#define LOG_PERFORMANCE 1

// These values will be used on the file as a log type identifier
// Ex:
// TIME LOG_NAME: <1> Log Description			// <1> -> Error log
// TIME LOG_NAME: <3> Log Description			// <3> -> Warning log
// The higher the value, more logs will be generated
#define BE_LOG_NO_LOGGING 0
#define BE_LOG_ERROR 1
#define BE_LOG_WARNING 3
#define BE_LOG_INFO 5
#define BE_LOG_VERBOSE 7
#define BE_LOG_PERFORMANCE 9
#define BE_LOG_ALL 128

#ifndef BE_LOG_SHOW_CALL_PLACE
	#ifdef _DEBUG
		#define BE_LOG_SHOW_CALL_PLACE
	#endif
#endif

#ifndef BE_LOG_ENABLE_PERFORMANCE
	#ifdef _DEBUG
		#define BE_LOG_ENABLE_PERFORMANCE 1
	#else
		#define BE_LOG_ENABLE_PERFORMANCE 1
	#endif
#endif

#ifndef LOG_LOGGING_THRESHOLD
	#ifdef _DEBUG
		#define LOG_LOGGING_THRESHOLD BE_LOG_ALL
	#else
		#define LOG_LOGGING_THRESHOLD BE_LOG_INFO
	#endif
#endif

#define LOG_IF_SHOULD_LOG(severity)                 \
    if (severity > LOG_LOGGING_THRESHOLD) ;  else   \

#define LOG_CLASS(output)														\
		private:																\
			void __getselfclassfunc__(){};										\
			static BitEngine::Logger& CL() {									\
				static BitEngine::Logger _log(GetClassName(&__getselfclassfunc__), output);			\
				return _log;													\
			}

#ifdef BE_LOG_SHOW_CALL_PLACE
	#define LOG(logger,severity)						\
        LOG_IF_SHOULD_LOG(severity)		                \
			BitEngine::LogLine(logger, severity) << __FUNCTION__ << ":" << __LINE__ << " | "
#else
	#define LOG(logger,severity)						\
        LOG_IF_SHOULD_LOG(severity)		                \
			BitEngine::LogLine(logger, severity)
#endif

#define LOGIFTRUE(logger,severity,expression)               \
    LOG_IF_SHOULD_LOG(severity)                             \
        if (expression){                                     \
            LOG(logger, severity) << ##expression is true!;}

#define LOGIFNULL(logger,severity,expression)               \
    LOG_IF_SHOULD_LOG(severity)                             \
        if (expression == nullptr){                          \
            LOG(logger, severity) << #expression "is NULL!";}

#define LOGCLASS(severity)							\
		LOG(CL(),severity)

#if BE_LOG_ENABLE_PERFORMANCE == 1
#define LOG_SCOPE_TIME(logto, description)		\
		BitEngine::ScopeLogger _log_scope(logto, description)
#define LOG_FUNCTION_TIME(logto)				\
		BitEngine::ScopeLogger _log_function(logto, __func__)
#else
#define LOG_SCOPE_TIME(logto, description)
#define LOG_FUNCTION_TIME(logto)
#endif

namespace BitEngine {


	class LogLine;

	class Logger
	{
		private:
			std::string logName;

			std::ofstream outFStream;
			std::ostream outStream;

		public:
			Logger(const std::string& name, std::ostream& output)
				: logName(name), outStream(output.rdbuf())
			{
				std::ostringstream str;
				str << header() << " LOG STARTED" << std::endl;
				outStream << str.str();
			}

			Logger(const std::string& name, Logger& output)
				: logName(name), outStream(output.getOutputSink().rdbuf())
			{
				std::ostringstream str;
				str << header() << " LOG STARTED" << std::endl;
				outStream << str.str();
			}

			Logger(const std::string& name, const char* file, std::ios_base::openmode mode)
				: logName(name), outFStream(file, mode), outStream(outFStream.rdbuf())
			{
				std::ostringstream str;
				str << header() << " LOG STARTED" << std::endl;
				outStream << str.str();
			}

			inline void Log(const std::string& line)
			{
				std::ostringstream str;
				str << header() << line;
				outStream << str.str();
			}

			std::ostream& getOutputSink() {
				return outStream;
			}

		private:
			inline std::string header()
			{
				std::ostringstream a;

				a << timeNowStr() << "(" << std::this_thread::get_id() << ") " << logName << ": ";

				return a.str();
			}

			inline static std::string timeNowStr()
			{
				char buff[32];
				tm sTm;

				time_t now = std::chrono::system_clock::to_time_t(Time::currentTime());
				localtime_s(&sTm, &now);
				strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S ", &sTm);
				return std::string(buff);
			}
	};

	class LogLine
	{
		public:
			LogLine(Logger& l, int severity)
				: log(l)
			{
				out << "<" << severity << "> - ";
			}

			~LogLine()
			{
				out << std::endl;
				log.Log(out.str());
			}

			template<typename T>
			inline std::ostringstream& operator << (const T& t)
			{
				out << t;
				return out;
			}

		private:
			std::ostringstream out;
			Logger& log;
	};

	class ScopeLogger
	{
		public:
			ScopeLogger(Logger& l, const std::string& descrp)
				: log(l), description(descrp)
			{
				timer.setTime();
			}

			~ScopeLogger()
			{
				double elapsed = timer.timeElapsedMs();
				BitEngine::LogLine(log, 9) << description << " took " << elapsed << " ms";
			}

		private:
			Logger& log;
			const std::string description;

			BitEngine::Timer timer;
	};

	extern Logger EngineLog;
}
