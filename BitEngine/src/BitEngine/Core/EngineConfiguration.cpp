#include "BitEngine/Core/EngineConfiguration.h"

#include <string.h>

#include "BitEngine/Core/Logger.h"

namespace BitEngine {

const char* LINE_COMMENT = "#";
const char* BLANK_SPACES = " \n\r\t";
const char SYSTEM_BEGIN_CHAR = '!';
const char CONFIG_VALUE_SEPARATOR_CHAR = ':';

EngineConfiguration::EngineConfiguration()
{
}

EngineConfiguration::~EngineConfiguration()
{
    for (auto& it : systemConfigs) {
        delete it.second;
    }
}

ConfigurationItem* EngineConfiguration::getConfiguration(const std::string& systemName, const std::string& configName, const std::string& defaultValue)
{
    BitEngine::SystemConfiguration* sc = getSystemConfiguration(systemName);
    if (sc != nullptr) {
        ConfigurationItem* value = sc->getConfig(configName);
        if (value == nullptr) {
            sc->addConfiguration(configName, "", defaultValue);
            return sc->getConfig(configName);
        }
        else {
            return value;
        }
    }
    else {
        sc = new SystemConfiguration(systemName);
        if (sc->addConfiguration(configName, "", defaultValue)) {
            systemConfigs[systemName] = sc;
            return sc->getConfig(configName);
        }
        else {
            return nullptr;
        }
    }
}

EngineConfigurationFileLoader::EngineConfigurationFileLoader(const std::string& fileName)
    : file(fileName)
{
}

void EngineConfigurationFileLoader::loadConfigurations(EngineConfiguration& ec) const
{
    std::ifstream inFile(file);

    // Save configs if couldn't open the file
    if (!inFile.is_open()) {
        saveConfigurations(ec);
        return;
    }

    // Read configs
    SystemConfiguration* workingSystem = nullptr;
    while (!inFile.eof()) {
        std::string line;
        line.resize(1024);

        /*auto& linestream = */ inFile.getline(&line[0], line.capacity());
        line.resize(strlen(line.c_str()));

        readLine(line, workingSystem, ec);
    }
}

void EngineConfigurationFileLoader::saveConfigurations(const EngineConfiguration& ec) const
{
    std::ofstream fileOut(file);

    const char* CONFIG_HEADER = "# Configuration file\n# Comment lines begin with #.\n"
                                "# Define a system config should have a line with !SystemName\n"
                                "# Configurations are set as:\n# ConfigName: value\n\n";

    if (!fileOut.is_open()) {
        LOG(EngineLog, BE_LOG_ERROR) << "EngineConfiguration: Failed to open configuration file!";
        return;
    }

    fileOut << CONFIG_HEADER;

    const std::map<std::string, SystemConfiguration*>& systemConfigs = ec.getConfigurations();
    for (const auto& it : systemConfigs) {
        const SystemConfiguration* sysConf = it.second;
        fileOut << "!" << it.first << std::endl;

        for (const auto& items : sysConf->getConfigs()) {
            const ConfigurationItem& item = items.second;
            fileOut << items.first << ": " << item.getValueAsString();

            // Include description
            if (!item.getDescription().empty())
                fileOut << " # " << item.getDescription() << std::endl;
        }
    }
}

int EngineConfigurationFileLoader::readLine(const std::string& line, SystemConfiguration*& workingSystem,
    EngineConfiguration& ec) const
{
    std::string workLine = line;

    // LOG(EngineLog, BE_LOG_VERBOSE) << "Line: '" << line << '\'';

    if (line.empty())
        return 0;

    if (!removeCommentsNTrim(workLine))
        return 0;

    if (workLine[0] == SYSTEM_BEGIN_CHAR) {
        auto nameEnd = workLine.find_first_of(BLANK_SPACES);

        std::string sysName = workLine.substr(1, nameEnd);

        workingSystem = ec.getSystemConfiguration(sysName);
        if (workingSystem == nullptr) {
            LOG(EngineLog, BE_LOG_WARNING) << "EngineConfiguration: system not found: '" << sysName << "'";
            return ERROR_UNKNOWN_SYSTEM;
        }
        else {
            LOG(EngineLog, BE_LOG_VERBOSE) << "EngineConfiguration: reading configuration for system: " << sysName;
            return 1;
        }
    }
    else { // May be a config

        // Find final character
        size_t end = workLine.find_last_of(CONFIG_VALUE_SEPARATOR_CHAR);
        if (end == workLine.size()) {
            LOG(EngineLog, BE_LOG_WARNING) << "EngineConfiguration: configuration not defined properly? Missing < " << CONFIG_VALUE_SEPARATOR_CHAR << " >";
            return ERROR_BAD_CONFIG;
        }

        std::string configName = workLine.substr(0, end);

        // It's a config item
        // Check if we are on a valid system
        if (workingSystem == nullptr) {
            LOG(EngineLog, BE_LOG_WARNING) << "EngineConfiguration: configuration is not defined for a system: " << configName;
            return ERROR_CONFIG_WITHOUT_SYS;
        }

        ConfigurationItem* configItem = workingSystem->getConfig(configName);
        if (configItem == nullptr) {
            LOG(EngineLog, BE_LOG_WARNING) << "Unknown configuration for system " << workingSystem->getSystemName() << " with name: " << configName;
            return ERROR_UNKNOWN_CONFIG;
        }

        // +1 so we ignore the ':'
        size_t begin = workLine.find_first_not_of(BLANK_SPACES, end + 1);
        if (begin != std::string::npos) {
            size_t end = workLine.find_first_of(BLANK_SPACES, begin);
            if (end == std::string::npos)
                end = begin - 1;
            const std::string configValue = workLine.substr(begin, end - begin);
            configItem->setValue(configValue);
            LOG(EngineLog, BE_LOG_INFO) << "CONFIG " << workingSystem->getSystemName() << ": " << configName << " set to '" << configValue << "'";
        }

        return 2;
    }
}

int EngineConfigurationFileLoader::removeCommentsNTrim(std::string& line) const
{
    auto pos = line.find_first_of(LINE_COMMENT);
    if (pos > 0) {
        // Get only the non-comment data
        line = line.substr(0, pos);
    }
    else if (pos == 0) {
        // Line starts with '#'
        // Full comment -> do nothing
        return 0;
    }

    // Find first valid character
    pos = line.find_first_not_of(BLANK_SPACES);
    // printf("pos: %d [%d,%d]\n", pos, line[0], line[1]);
    if (pos == line.size()) {
        return 0;
    }
    line = line.substr(pos);

    // Ignore if empty
    if (line.empty())
        return 0;

    // Line should be processed
    return 1;
}

} // bitengine
