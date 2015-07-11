#pragma once

#include <iostream>
#include <fstream>

// TODO: thread safe with std::recursive_mutex

#define LOG_SEVERITY_VERBOSE		0		// Show ALL
#define LOG_SEVERITY_INFORMATION	1
#define LOG_SEVERITY_ERROR			2
#define LOG_SEVERITY_NO_LOGS		100

#define NEW_LOG(id, output, name, severity, minServ) \
	BitEngine::Logger<id> BitEngine::Logger<id>::self(output, name, severity, minServ)

#define NEW_CONSOLE_LOG(id, name, severity, minServ) \
		BitEngine::Logger<id> BitEngine::Logger<id>::self(name, severity, minServ)

#define LOGTO(id) \
		((BitEngine::Logger<id>::self))
#define LOG() \
		((BitEngine::Logger<0>::self))

#define SHOULD_LOG(x, xMax)	\
		(x != LOG_SEVERITY_NO_LOGS) && (xMax <= x)

// #define NEW_LOG(id, output, name) /##/
// #define NEW_CONSOLE_LOG(id, name) /##/
// 
// #define LOGTO(id) /##/
// #define LOG() /##/


namespace BitEngine{

struct endlog_t{};
static endlog_t endlog;

template<int instance>
class Logger
{
private:
	std::streambuf *buf;
	std::ofstream of;
	std::ostream out;

	std::string name;

	bool m_begining;
	const int severity;
	const int logLimit;

public:
	static Logger self;

	Logger(const char* _name, int _sev, int minSev)
		: out(nullptr), m_begining(true), severity(_sev), logLimit(minSev)
	{
		if (SHOULD_LOG(severity, logLimit)){
			name = _name;
			buf = std::cout.rdbuf();
			out.rdbuf(buf);
		}
	}

	Logger(const char* file, const char* _name, int _serv, int minSev)
		: out(nullptr), m_begining(true), severity(_sev), logLimit(minSev)
	{
		if (SHOULD_LOG(severity, logLimit)){
			name = _name;

			if (file == 0){
				buf = std::cout.rdbuf();
			}
			else {
				of.open(file);
				buf = of.rdbuf();
			}

			out.rdbuf(buf);
		}
	}

	Logger& operator<<(const endlog_t& endl)
	{
		if (SHOULD_LOG(severity, logLimit)){
			m_begining = true;
			out << "\n";
		}
		return *this;
	}

	template<typename T>
	Logger& operator << (const T& t){
		if (SHOULD_LOG(severity, logLimit)){
			if (m_begining){
				out << "<" << severity << "> " << name << ": ";
				m_begining = false;
			}

			out << t;
		}
		return *this;
	}
};


}
