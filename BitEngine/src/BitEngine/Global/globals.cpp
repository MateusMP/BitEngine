#include <BitEngine/Core/Logger.h>
#include <BitEngine/Core/api.h>
#include <BitEngine/Core/Profiler.h>

/**
 * All global variables are defined in this file.
 * This file should be added once per project, in the final linked unit.
 * Optionally, one may override definitions here with other standard values.
 * */

namespace BitEngine {

    BE_API Logger* EngineLog = nullptr;

    namespace Profiling 
    {        
        BE_API ChromeProfiler* _instance = nullptr;

        BE_API ChromeProfiler& Get()
        {
            return *_instance;
        }

        BE_API void SetInstance(ChromeProfiler* obj)
        {
            _instance = obj;
        }
    }
}
