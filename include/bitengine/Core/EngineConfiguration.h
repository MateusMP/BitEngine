#pragma once

#include <fstream>

#include "bitengine/Common/ErrorCodes.h"
#include "bitengine/Core/GameEngine.h"
#include "bitengine/Core/SystemConfiguration.h"

namespace BitEngine
{
	class EngineConfiguration : public EnginePiece
	{
	public:
		EngineConfiguration(GameEngine* ge, const std::string& fileName);
		~EngineConfiguration();
		
		void LoadConfigurations();
		void SaveConfigurations();

		ConfigurationItem* getConfiguration(const std::string& systemName, const std::string& configName, const std::string& defaultValue);

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
