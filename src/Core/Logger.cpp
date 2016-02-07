#include "Core/Logger.h"

#include <iostream>

namespace BitEngine {

#if defined(_DEBUG) || defined(BE_LOG_FORCE_OUTPUT_CONSOLE)
	Logger EngineLog("EngineLog", std::cout);
#else
	std::ofstream file("EngineLog.log", std::ios_base::app);
	Logger EngineLog("EngineLog", file);
#endif

}
