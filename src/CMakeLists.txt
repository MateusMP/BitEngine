add_subdirectory(Core)
add_subdirectory(DefaultBackends)

add_library(bitengine STATIC 
					$<TARGET_OBJECTS:be_core>
					$<TARGET_OBJECTS:be_ecs>
					$<TARGET_OBJECTS:be_graphics>
					$<TARGET_OBJECTS:be_resources>)