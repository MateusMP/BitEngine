#include "BitEngine/Core/Logger.h"

#include <iostream>
#include <string.h>

namespace BitEngine {

/*
std::ofstream file("EngineLog.log", std::ios_base::app);
#if defined(_DEBUG) || defined(BE_LOG_FORCE_OUTPUT_CONSOLE)
	Logger EngineLog("EngineLog", {&file, &std::cout});
#else
	Logger EngineLog("EngineLog", file);
#endif
*/
	// By default, the logger will log to the console
	BE_API Logger* EngineLog = nullptr;
	LoggerSetup LoggerSetup::loggerSetup;

	bool containsOption(const char* option, int argc, const char* argv[]) {
		for (int i = 0; i < argc; ++i) {
			if (strcmp(option, argv[i]) == 0) {
				return true;
			}
		}
		return false;
	}

	void LoggerSetup::Setup(int argc, const char* argv[]) {

		if (loggerSetup.initialized) {
			throw std::runtime_error("Logger already initiliazed");
		}
		else
		{
			if (EngineLog != nullptr) {
				delete EngineLog;
				EngineLog = nullptr;
			}

			loggerSetup.file.open("EngineLog.log", std::ios_base::app);

			if (containsOption("--debug", argc, argv)
			&& !containsOption("--debug-file-only", argc, argv)) {
				BitEngine::EngineLog = new Logger("EngineLog", {&loggerSetup.file, &std::cout});
			} else {
				BitEngine::EngineLog = new Logger("EngineLog", {&loggerSetup.file});
			}

			loggerSetup.initialized = true;
		}
	}

	LoggerSetup::LoggerSetup()
	{
	}

	LoggerSetup::~LoggerSetup() {
		if (initialized) {
			delete EngineLog;
			EngineLog = nullptr;
			initialized = false;
		}
	}

}
