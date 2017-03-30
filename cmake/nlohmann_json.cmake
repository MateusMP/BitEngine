if (NEED_NLOHMANN_JSON)
	find_package(nlohmann_json PATHS ${CMAKE_INSTALL_PREFIX}/cmake NO_DEFAULT_PATH)
	
	if (NOT ${JSON_INCLUDE_DIR} STREQUAL "")
		message(STATUS "nlohmann_json FOUND!")
	else()

		ExternalProject_Add(nlohmann_json_dep
			PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/nlohmann_json
			GIT_REPOSITORY https://github.com/nlohmann/json.git
			UPDATE_COMMAND ""
			CMAKE_ARGS
				-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
				-DBuildTests=OFF
		)
		#set(NLOHJSON_INCLUDE ${CMAKE_INSTALL_PREFIX}/include/nlohmann)
		#message(STATUS "NLOHJSON_INCLUDE='${NLOHJSON_INCLUDE}'")
	endif()	
	
endif()