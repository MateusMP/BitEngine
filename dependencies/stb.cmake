if (NEED_STB)	
    ExternalProject_Add(stb_dep
        PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
    )
    
    # Recover project paths for additional settings.
    ExternalProject_Get_Property(stb_dep SOURCE_DIR)
    set(STB_INCLUDE ${SOURCE_DIR})
    message(STATUS "STB_INCLUDE='${STB_INCLUDE}'")
    
endif()
