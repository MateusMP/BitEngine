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
		const char* LINE_COMMENT = "#";
		const char* BLANK_SPACES = " \n\r\t";
		const char SYSTEM_BEGIN_CHAR = '!';
		const char CONFIG_VALUE_SEPARATOR_CHAR = ':';

		EngineConfiguration(const std::string& fileName);

		bool AddConfiguration(const std::string& systemName, SystemConfiguration* sysConfig);

		void LoadConfigurations();

		void SaveConfigurations();

	private:
		std::string file;
		std::map<std::string, SystemConfiguration*> systemConfigs;

		/**
		  * @returns: ERROR_UNKNOWN_SYSTEM	
		  *			  ERROR_UNKNOWN_CONFIG	
		  *			  ERROR_BAD_CONFIG		
		  *			  ERROR_CONFIG_WITHOUT_SYS
		  */
		int readLine(const std::string& line, std::map<std::string, SystemConfiguration*>::iterator& workingSystem);

		/**
		 * @returns: 0 -> stop processing line
		 */
		int removeCommentsNTrim(std::string& line);
	};


}