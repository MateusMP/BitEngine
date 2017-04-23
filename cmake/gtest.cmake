if (BUILD_TESTS)

    list(APPEND DEPENDENCIES googletest)

    if (MSVC)
        set(BUILD_GTEST_MSVC_WITH_CRT -Dgtest_force_shared_crt:INTERNAL=ON)
    endif()

    ExternalProject_Add(googletest
        PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/gtest
        GIT_REPOSITORY    https://github.com/google/googletest.git
        GIT_TAG           aa148eb2b7f70ede0eb10de34b6254826bfb34f4
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            ${BUILD_GTEST_MSVC_WITH_CRT}
            -DBUILD_SHARED_LIBS=OFF
        TEST_COMMAND    ""
        UPDATE_COMMAND  ""
        # gtest_force_shared_crt ON CACHE BOOL "" FORCE
    )
    set(GTEST_INCLUDE ${CMAKE_INSTALL_PREFIX}/include)
    set(GTEST_LIBRARIES 
                ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}
                ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}gmock${CMAKE_STATIC_LIBRARY_SUFFIX})
    message(STATUS "GTEST_INCLUDE='${GTEST_INCLUDE}'")
    message(STATUS "GTEST_LIBRARIES='${GTEST_LIBRARIES}'")

    
endif()
