
add_executable(TestBuild beTestMain.cpp
	test_build.cpp)
add_test(BuildTests TestBuild)
target_link_libraries(TestBuild ${GTEST_LIBRARIES} -static-libstdc++ bitengine)
target_include_directories(TestBuild PUBLIC ${GTEST_INCLUDE} ${ALL_BE_INCLUDES})

add_executable(TestCore beTestMain.cpp
		Core/reflectiontest.cpp
		Core/Resource/resourceLoaderTests.cpp)
add_test(CoreTests TestCore WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
target_link_libraries(TestCore bitengine ${GTEST_LIBRARIES} -static-libstdc++ bitengine)
target_include_directories(TestCore PUBLIC ${GTEST_INCLUDE} ${ALL_BE_INCLUDES})

add_executable(TestCommonUtils beTestMain.cpp
		Common/bitsetTests.cpp
		Common/commonTests.cpp
		Common/vectorBoolTests.cpp)
add_test(CommonTests TestCommonUtils)
target_link_libraries(TestCommonUtils ${GTEST_LIBRARIES} -static-libstdc++ bitengine)
target_include_directories(TestCommonUtils PUBLIC ${GTEST_INCLUDE} ${ALL_BE_INCLUDES})


file(COPY ../resources DESTINATION .)