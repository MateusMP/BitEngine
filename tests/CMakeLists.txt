
add_executable(TestBuild 
	test_build.cpp)
add_test(BuildTests TestBuild)
target_link_libraries(TestBuild ${GTEST_LIBRARIES} -static-libstdc++)
target_include_directories(TestBuild PUBLIC ${GTEST_INCLUDE} ${ALL_BE_INCLUDES})

add_executable(TestCore
		Core/reflectiontest.cpp)
add_test(ReflectionTests TestCore)
target_link_libraries(TestCore bitengine ${GTEST_LIBRARIES} -static-libstdc++)
target_include_directories(TestCore PUBLIC ${GTEST_INCLUDE} ${ALL_BE_INCLUDES})

add_executable(TestCommonUtils
		Common/bitsetTests.cpp
		Common/commonTests.cpp
		Common/vectorBoolTests.cpp)
add_test(TestCommon TestCommonUtils)
target_link_libraries(TestCommonUtils ${GTEST_LIBRARIES} -static-libstdc++)
target_include_directories(TestCommonUtils PUBLIC ${GTEST_INCLUDE} ${ALL_BE_INCLUDES})
