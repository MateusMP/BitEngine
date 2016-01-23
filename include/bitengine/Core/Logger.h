#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>

#include "Timer.h"

//#define LOG_PERFORMANCE 1

#ifndef BE_LOG_ENABLE_PERFORMANCE
#define BE_LOG_ENABLE_PERFORMANCE 0
#endif

// These values will be used on the file as a log type identifier
// Ex: 
// TIME LOG_NAME: <1> Log Description			// <1> -> Error log
// TIME LOG_NAME: <3> Log Description			// <3> -> Warning log
#define BE_LOG_NO_LOGGING 0
#define BE_LOG_ERROR 1
#define BE_LOG_WARNING 3
#define BE_LOG_INFO 5
#define BE_LOG_VERBOSE 7
#define BE_LOG_PERFORMANCE 9
#define BE_LOG_ALL 128

// TODO: thread safe with std::recursive_mutex

// The higher the value, more logs will be generated

#ifndef LOG_LOGGING_THRESHOLD
#define LOG_LOGGING_THRESHOLD BE_LOG_ALL
#endif

#define LOG_STATIC(logname, output)									\
		public:														\
			static BitEngine::Logger& SELFLOG() {					\
				static BitEngine::Logger _log(logname, output);	\
				return _log;										\
			}														\
		private:

#define LOG(logger,severity)						\
		if (severity > LOG_LOGGING_THRESHOLD) ;		\
		else BitEngine::LogLine(logger, severity)

#define LOG_SCOPE_TIME(logto, description)		\
		if (LOG_PERFORMANCE)					\
			BitEngine::ScopeLogger(logto, description)

#define LOG_FUNCTION_TIME(logto)				\
			BitEngine::ScopeLogger _log_function(logto, __func__)

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
				str << timeNowStr() << logName << " LOG STARTED" << std::endl;
				outStream << str.str();
			}

			Logger(const std::string& name, const char* file, int mode)
				: logName(name), outFStream(file, mode), outStream(outFStream.rdbuf())
			{
				std::ostringstream str;
				str << timeNowStr() << logName << " LOG STARTED" << std::endl;
				outStream << str.str();
			}

			inline void Log(const std::string& line)
			{
				std::ostringstream str;
				str << timeNowStr() << logName << ": " << line;
				outStream << str.str();
			}

			inline static time_t timeNow()
			{
				return time(0);
			}

			inline static std::string timeNowStr()
			{
				char buff[32];
				tm sTm;

				time_t now = timeNow();
				gmtime_s(&sTm, &now);

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
				double elapsed = timer.timeElapsedSeconds();
				BitEngine::LogLine(log, 9) << description << " took " << elapsed << " seconds";
			}

		private:
			Logger& log;
			const std::string description;

			BitEngine::Timer timer;
	};

	extern Logger EngineLog;
}
