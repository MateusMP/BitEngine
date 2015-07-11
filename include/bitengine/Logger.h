#pragma once

#include <iostream>
#include <fstream>

// TODO: thread safe with std::recursive_mutex

#ifdef _DEBUG

#define NEW_LOG(id, output, name) \
	BitEngine::Logger<id> BitEngine::Logger<id>::self(output, name)
#define NEW_CONSOLE_LOG(id, name) \
	BitEngine::Logger<id> BitEngine::Logger<id>::self(name)

#define LOGTO(id) \
	((BitEngine::Logger<id>::self))
#define LOG() \
	((BitEngine::Logger<0>::self))
#else
#define NEW_LOG(id, output, name) /##/
#define NEW_CONSOLE_LOG(id, name) /##/

#define LOGTO(id) /##/
#define LOG() /##/
#endif

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

public:
	static Logger self;

	Logger(const char* _name)
		: out(nullptr), m_begining(true)
	{
		name = _name;
		buf = std::cout.rdbuf();
		out.rdbuf(buf);
	}

	Logger(const char* file, const char* _name)
		: out(nullptr), m_begining(true)
	{
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

	Logger& operator<<(const endlog_t& endl)
	{
		m_begining = true;
		out << "\n";

		return *this;
	}

	template<typename T>
	Logger& operator << (const T& t){
		if (m_begining){
			out << name << ": ";
			m_begining = false;
		}

		out  << t;
		return *this;
	}
};


}
