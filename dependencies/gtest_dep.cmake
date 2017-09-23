if (BUILD_TESTS)

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
