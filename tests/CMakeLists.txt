set(GTEST_LIBRARIES )

INCLUDE_DIRECTORIES(
	../dependencies/googletest/googletest/include
	../dependencies/googletest/googletest/
	../dependencies/googletest/googlemock/include
	../dependencies/googletest/googlemock/
)

set(TEST_SRCS ../dependencies/googletest/googletest/src/gtest-all.cc
			  ../dependencies/googletest/googlemock/src/gmock-all.cc)

add_executable(TestCore beTestMain.cpp ${TEST_SRCS}
		test_build.cpp
		Core/reflectiontest.cpp
		Core/Resource/resourceLoaderTests.cpp
		Common/bitsetTests.cpp
		Common/commonTests.cpp
		Common/vectorBoolTests.cpp
)
target_link_libraries(TestCore ${GTEST_LIBRARIES} bitengine)
target_include_directories(TestCore PUBLIC ${GTEST_INCLUDE})
add_test(CoreTests TestCore WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
