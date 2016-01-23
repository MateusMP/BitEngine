#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>

#define NO_LOGGING 0
#define ERROR 1
#define WARNING 3
#define INFO 5
#define VERBOSE 7
#define LOG_ALL 128

// TODO: thread safe with std::recursive_mutex

// The higher the value, more logs will be generated

#ifndef LOG_LOGGING_THRESHOLD
#define LOG_LOGGING_THRESHOLD LOG_ALL
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
				str << timeNow() << logName << " LOG STARTED" << std::endl;
				outStream << str.str();
			}

			Logger(const std::string& name, const char* file, int mode)
				: logName(name), outFStream(file, mode), outStream(outFStream.rdbuf())
			{
				std::ostringstream str;
				str << timeNow() << logName << " LOG STARTED" << std::endl;
				outStream << str.str();
			}

			inline void Log(const std::string& line)
			{
				std::ostringstream str;
				str << timeNow() << logName << ": " << line;
				outStream << str.str();
			}

		private:
			inline static std::string timeNow()
			{
				char buff[32];
				struct tm sTm;
				time_t now = time(0);
				gmtime_s(&sTm, &now);

				strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S ", &sTm);
				return std::string(buff);
			}
	};

	class LogLine
	{
		friend class Logger;

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

	extern Logger EngineLog;
}
