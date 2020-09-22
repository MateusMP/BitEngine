#pragma once

#include <fstream>

#include "BitEngine/Common/ErrorCodes.h"
#include "BitEngine/Core/SystemConfiguration.h"

// Param will cause log to be sent to the console and the default file
#define BE_PARAM_DEBUG "--debug"
// Log only to file
#define BE_PARAM_DEBUG_FILE_ONLY "--debug-file-only"

namespace BitEngine {
class BE_API EngineConfiguration {
public:
    EngineConfiguration();
    ~EngineConfiguration();

    ConfigurationItem* getConfiguration(const std::string& systemName, const std::string& configName, const std::string& defaultValue);

    SystemConfiguration* getSystemConfiguration(const std::string& systemName) const
    {
        auto it = systemConfigs.find(systemName);
        if (it == systemConfigs.end()) {
            return nullptr;
        }
        else {
            return it->second;
        }
    }

    const std::map<std::string, SystemConfiguration*>& getConfigurations() const
    {
        return systemConfigs;
    }

private:
    std::map<std::string, SystemConfiguration*> systemConfigs;
};

class ConfigurationLoader {
public:
    virtual ~ConfigurationLoader() = 0;
    virtual void loadConfigurations(EngineConfiguration& ec) const = 0;
    virtual void saveConfigurations(const EngineConfiguration& ec) const = 0;
};
inline ConfigurationLoader::~ConfigurationLoader(){};

class EngineConfigurationFileLoader : public ConfigurationLoader {
public:
    EngineConfigurationFileLoader(const std::string& fileName);
    ~EngineConfigurationFileLoader() {}

    void loadConfigurations(EngineConfiguration& ec) const;
    void saveConfigurations(const EngineConfiguration& ec) const;

private:
    /**
			  * @returns: ERROR_UNKNOWN_SYSTEM
			  *			  ERROR_UNKNOWN_CONFIG
			  *			  ERROR_BAD_CONFIG
			  *			  ERROR_CONFIG_WITHOUT_SYS
			  */
    int readLine(const std::string& line, SystemConfiguration*& workingSystem,
        EngineConfiguration& systemConfigs) const;

    /**
			 * @returns: 0 -> stop processing line
			 */
    int removeCommentsNTrim(std::string& line) const;

    std::string file;
};
}
