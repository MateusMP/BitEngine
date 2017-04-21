
message(STATUS " bitengine depends on ${DEPENDENCIES}")

ExternalProject_Add (bitengine
  DEPENDS ${DEPENDENCIES}
  PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/bitengine
  SOURCE_DIR ${PROJECT_SOURCE_DIR}
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/bitengine_sb
  CMAKE_ARGS
	-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
	-DBUILD_DEPENDENCIES=OFF
        -DBE_BASE_DEPENDENCY_DIRECTORY=${BE_BASE_DEPENDENCY_DIRECTORY}
)

enable_testing()
add_test(NAME bitengine_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bitengine_sb
        COMMAND ${CMAKE_CTEST_COMMAND} --verbose)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose)
