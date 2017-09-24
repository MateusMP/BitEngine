if (NEED_STB)	
    list(APPEND DEPENDENCIES stb)
    find_package(stb PATHS ${CMAKE_INSTALL_PREFIX}/lib/cmake/stb NO_DEFAULT_PATH QUIET)
    
    if (NOT TARGET stb)
        add_library(stb STATIC ${BE_BASE_DEPENDENCY_DIRECTORY}/stb/src/stb_dep/stb_image.h)
        set_target_properties(stb PROPERTIES LINKER_LANGUAGE CXX)
        target_include_directories(stb INTERFACE  $<BUILD_INTERFACE:${BE_BASE_DEPENDENCY_DIRECTORY}/stb/src/stb_dep>
												  $<INSTALL_INTERFACE:include/>)
        install(TARGETS stb DESTINATION include/ EXPORT bitengineTargets)
    endif()
    
    set(STB_INCLUDE ${BE_BASE_DEPENDENCY_DIRECTORY}/stb/src/stb_dep)
endif()
