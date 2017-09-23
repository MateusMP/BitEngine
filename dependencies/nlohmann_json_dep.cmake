if (NEED_NLOHMANN_JSON)

	find_package(nlohmann_json PATHS ${CMAKE_INSTALL_PREFIX}/cmake NO_DEFAULT_PATH QUIET)
	list(APPEND DEPENDENCIES nlohmann_json)
	
endif()
