#include "Core/Logger.h"

#include <iostream>

namespace BitEngine {
#ifdef _DEBUG
	Logger EngineLog("EngineLog", std::cout);
#else
	std::ofstream file("EngineLog.log", std::ios_base::app);
	Logger EngineLog("EngineLog", file);
#endif
}