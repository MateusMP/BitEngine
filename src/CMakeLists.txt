add_subdirectory(Core)
add_subdirectory(DefaultBackends)

add_library(BitEngine .)
target_link_libraries(BitEngine core)