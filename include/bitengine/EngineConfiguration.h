#pragma once

#include <fstream>

#include "ErrorCodes.h"
#include "EngineLoggers.h"

#include "SystemConfiguration.h"

namespace BitEngine
{
	class EngineConfiguration
	{
	public:
		EngineConfiguration(const std::string& fileName)
			: file(fileName)
		{
		}

		bool AddConfiguration(const std::string& systemName, SystemConfiguration* sysConfig){
			return systemConfigs.emplace(systemName, sysConfig).second;
		}

		void LoadConfigurations(){
			std::ifstream inFile(file);

			// Save configs if couldn't open the file
			if (!inFile.is_open()){
				SaveConfigurations();
				return;
			}

			// Read configs
			std::map<std::string, SystemConfiguration*>::iterator workingSystem = systemConfigs.end();
			while (!inFile.eof())
			{
				std::string line;
				line.resize(1024);

				auto& linestream = inFile.getline(&line[0], line.capacity());
				line.resize(strlen(line.c_str()));

				readLine(line, workingSystem);
			}
		}

		void SaveConfigurations()
		{
			std::ofstream fileOut(file);

			const char* CONFIG_HEADER = "# Configuration file\n# Comment lines begin with #.\n"				\
										"# Define a system config should have a line with !SystemName\n"	\
										"# Configurations are set as:\n# ConfigName: value\n\n";

			if (!fileOut.is_open()){
				LOGTO(Error) << "EngineConfiguration: Failed to open configuration file!" << endlog;
				return;
			}

			fileOut << CONFIG_HEADER;

			for (const auto& it : systemConfigs)
			{
				const SystemConfiguration* sysConf = it.second;
				fileOut << "!" << it.first << std::endl;

				for (const auto& items : sysConf->getConfigs()){
					const ConfigurationItem& item = items.second;
					fileOut << items.first << ": " << item.getValueAsString();

					// Include description
					if (!item.getDescription().empty())
						fileOut << " # " << item.getDescription() << std::endl;
				}
			}
		}

	private:
		std::string file;
		std::map<std::string, SystemConfiguration*> systemConfigs;

		/**
		  * @returns: ERROR_UNKNOWN_SYSTEM	
		  *			  ERROR_UNKNOWN_CONFIG	
		  *			  ERROR_BAD_CONFIG		
		  *			  ERROR_CONFIG_WITHOUT_SYS
		  */
		int readLine(const std::string& line, std::map<std::string, SystemConfiguration*>::iterator& workingSystem)
		{
			const char* LINE_COMMENT = "#";
			const char* BLANK_SPACES = " \n\r\t";
			const char SYSTEM_BEGIN_CHAR = '!';
			const char CONFIG_VALUE_SEPARATOR_CHAR = ':';
			
			std::string workLine = line;

			// LOGTO(Verbose) << "Line: '" << line << '\'' << endlog;

			if (line.empty())
				return 0;

			auto pos = line.find_first_of(LINE_COMMENT);
			if (pos > 0){
				// Get only the non-comment data
				workLine = line.substr(0, pos);
			}
			else if (pos == 0){
				// Line starts with '#'
				// Full comment -> do nothing
				return 0;
			}

			// Find first valid character
			pos = line.find_first_not_of(BLANK_SPACES);
			// printf("pos: %d [%d,%d]\n", pos, line[0], line[1]);
			if (pos == line.size()){
				return 0;
			}
			workLine = line.substr(pos);
			
			// Ignore if empty
			if (workLine.empty())
				return 0;

			if (workLine[0] == SYSTEM_BEGIN_CHAR)
			{
				auto nameEnd = workLine.find_first_of(BLANK_SPACES);

				std::string sysName = workLine.substr(1, nameEnd);

				workingSystem = systemConfigs.find(sysName);
				if (workingSystem == systemConfigs.end()){
					LOGTO(Warning) << "EngineConfiguration: system not found: '" << sysName << "'" << endlog;
					return ERROR_UNKNOWN_SYSTEM;
				}
				else
				{
					LOGTO(Verbose) << "EngineConfiguration: reading configuration for system: " << sysName << endlog;
					return 1;
				}
			}
			else { // May be a config

				auto begin = workLine.find_first_not_of(BLANK_SPACES);

				// Ignore blank space
				if (begin != 0){
					workLine = workLine.substr(begin);
				}

				// Do nothing if empty line was found
				if (workLine.empty())
					return 0;

				// Find final character
				auto end = workLine.find_last_of(CONFIG_VALUE_SEPARATOR_CHAR);
				if (end == workLine.size()){
					LOGTO(Warning) << "EngineConfiguration: configuration not defined properly? Missing < " << CONFIG_VALUE_SEPARATOR_CHAR << " >" << endlog;
					return ERROR_BAD_CONFIG;
				}

				std::string configName = workLine.substr(0, end);

				// It's a config item
				// Check if we are on a valid system
				if (workingSystem == systemConfigs.end()){
					LOGTO(Warning) << "EngineConfiguration: configuration is not defined for a system: " << configName << endlog;
					return ERROR_CONFIG_WITHOUT_SYS;
				}

				ConfigurationItem* configItem = workingSystem->second->getConfig(configName);
				if (configItem == nullptr){
					LOGTO(Warning) << "Unknown configuration for system " << workingSystem->first << " with name: " << configName << endlog;
					return ERROR_UNKNOWN_CONFIG;
				}

				// +1 so we ignore the ':'
				begin = workLine.find_first_not_of(BLANK_SPACES, end + 1);
				std::string configValue = workLine.substr(begin, workLine.find_first_of(BLANK_SPACES, begin));

				configItem->setValue(configValue);

				LOGTO(Info) << "CONFIG " << workingSystem->first << ": " << configName << " set to '" << configValue << "'" << endlog;
				return 2;
			}

		}
	};


}