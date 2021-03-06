#pragma once

#include "api.h"

#include <string>
#include <vector>
#include <map>

namespace BitEngine {
class BE_API ConfigurationItem {
public:
    ConfigurationItem(const std::string& _name,
        const std::string& _defaultValue,
        const std::string& _description)
        : name(_name)
        , defaultValue(_defaultValue)
        , value(_defaultValue)
        , description(_description)
    {
    }

    void setValue(const std::string& newValue)
    {
        value = newValue;
    }

    void setValue(double newValue)
    {
        value = std::to_string(newValue);
    }

    void setValue(bool newValue)
    {
        if (newValue) {
            value = "true";
        }
        else {
            value = "false";
        }
    }

    double getValueAsReal() const
    {
        return std::strtod(value.c_str(), nullptr);
    }

    bool getValueAsBool() const
    {
        return ((value.compare("true") == 0) || (value.compare("1") == 0));
    }

    const std::string& getValueAsString() const
    {
        return value;
    }

    const std::string& getDefaultValue() const
    {
        return defaultValue;
    }

    double getDefaultValueAsReal() const
    {
        return std::strtod(defaultValue.c_str(), nullptr);
    }

    const std::string& getDescription() const
    {
        return description;
    }

    void setDescription(const std::string& str)
    {
        description = str;
    }

private:
    std::string name;
    std::string defaultValue;
    std::string value;
    std::string description;
};

class BE_API SystemConfiguration {
public:
    SystemConfiguration(const std::string& name)
        : systemName(name)
    {
    }

    const std::map<std::string, ConfigurationItem>& getConfigs() const
    {
        return configs;
    }

    bool addConfiguration(const std::string& name, const std::string& description, const std::string& defaultValue = "")
    {
        return configs.insert(std::pair<std::string, ConfigurationItem>(name, ConfigurationItem(name, defaultValue, description))).second;
    }

    ConfigurationItem* getConfig(const std::string& name)
    {
        auto it = configs.find(name);
        if (it == configs.end())
            return nullptr;

        return &it->second;
    }

    void resetToDefaults()
    {
        for (auto& it : configs) {
            it.second.setValue(it.second.getDefaultValue());
        }
    }

    const std::string& getSystemName() const
    {
        return systemName;
    }

private:
    std::string systemName;
    std::map<std::string, ConfigurationItem> configs;
};
}
