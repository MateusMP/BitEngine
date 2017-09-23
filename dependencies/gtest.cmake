if (BUILD_TESTS)

    list(APPEND DEPENDENCIES googletest_dep)

    if (MSVC)
        set(BUILD_GTEST_MSVC_WITH_CRT -Dgtest_force_shared_crt:INTERNAL=ON)
    endif()

    ExternalProject_Add(googletest_dep
        PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/gtest
        GIT_REPOSITORY    https://github.com/google/googletest.git
        GIT_TAG           780bae0facea90a5b2105cbc09e87d99887c2e23
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            ${BUILD_GTEST_MSVC_WITH_CRT}
            -DBUILD_SHARED_LIBS=OFF
        TEST_COMMAND    ""
        UPDATE_COMMAND  ""
        # gtest_force_shared_crt ON CACHE BOOL "" FORCE
    )
    
    # Since gtest does not provide a config file, we can't find the libraries properly so we
    # hardcode this prefixes assuming they wont change...
    set(LIB_PRE lib)
    set(LIB_SUF .a)
    set(GTEST_INCLUDE ${CMAKE_INSTALL_PREFIX}/include)
    set(GTEST_LIBRARIES 
                ${CMAKE_INSTALL_PREFIX}/lib/${LIB_PRE}gtest${LIB_SUF}
                ${CMAKE_INSTALL_PREFIX}/lib/${LIB_PRE}gmock${LIB_SUF})
    message(STATUS "GTEST_INCLUDE='${GTEST_INCLUDE}'")
    message(STATUS "GTEST_LIBRARIES='${GTEST_LIBRARIES}'")

    
endif()
