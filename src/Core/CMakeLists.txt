# root
set(HEADERS
	../../include/bitengine/bitengine.h
	../../include/bitengine/Common/BitFieldVector.h
	../../include/bitengine/Common/MacroHelpers.h
	../../include/bitengine/Common/ThreadSafeQueue.h
	../../include/bitengine/Common/VectorBool.h
	../../include/bitengine/Common/ErrorCodes.h
	../../include/bitengine/Common/MathUtils.h
	../../include/bitengine/Common/TypeDefinition.h

	../../include/bitengine/Core/Assert.h
	../../include/bitengine/Core/Graphics.h
	../../include/bitengine/Core/Messenger.h
	../../include/bitengine/Core/Task.h
	../../include/bitengine/Core/CommandSystem.h
	../../include/bitengine/Core/Input.h
	../../include/bitengine/Core/ModelManager.h
	../../include/bitengine/Core/TaskManager.h
	../../include/bitengine/Core/InputSystem.h
	../../include/bitengine/Core/Reflection.h
	../../include/bitengine/Core/Timer.h
	../../include/bitengine/Core/EngineConfiguration.h  
	../../include/bitengine/Core/Logger.h       
	../../include/bitengine/Core/VideoSystem.h
	../../include/bitengine/Core/GameEngine.h           
	../../include/bitengine/Core/Math.h         
	../../include/bitengine/Core/ResourceSystem.h       
	../../include/bitengine/Core/Window.h
	../../include/bitengine/Core/Memory.h       
	../../include/bitengine/Core/Signal.h
	../../include/bitengine/Core/GeneralTaskManager.h   
	../../include/bitengine/Core/Mesh.h         
	../../include/bitengine/Core/System.h
	../../include/bitengine/Core/SystemConfiguration.h
)

set(SRCS
	CommandSystem.cpp
	EngineConfiguration.cpp
	GeneralTaskManager.cpp
    Input.cpp
	Logger.cpp
    ModelManager.cpp
	Reflection.cpp
    ResourceSystem.cpp
    Timer.cpp
    VideoRenderer.cpp	
	Messenger.cpp
    
)
# Resources
set(SRCS ${SRCS}
	Resources/DevResourceLoader.cpp
	Resources/SpriteManager.cpp
	Resources/ResourceManager.cpp
)
# Graphics
set(SRCS ${SRCS}
	Graphics/Sprite2D.cpp
)

set(SRCS ${SRCS}
	ECS/BaseEntitySystem.cpp
	ECS/Camera2DComponent.cpp
	ECS/Camera2DProcessor.cpp
	ECS/Camera3DComponent.cpp
	ECS/Camera3DProcessor.cpp
	ECS/ComponentProcessor.cpp
	ECS/GameLogicProcessor.cpp
	ECS/RenderableMeshProcessor.cpp
	ECS/Transform2DComponent.cpp
	ECS/Transform2DProcessor.cpp
	ECS/Transform3DComponent.cpp
	ECS/Transform3DProcessor.cpp
)

message(STATUS "BITENGINE_INCLUDE_DIR='${BITENGINE_INCLUDE_DIR}'")

# Create the bitengine lib
add_library(bitengine ${SRCS} ${BITENGINE_INCLUDE_FILES} ${HEADERS})
SET_TARGET_PROPERTIES(bitengine PROPERTIES LINKER_LANGUAGE CXX)

if (CMAKE_COMPILER_IS_GNUCC)
	target_link_libraries(bitengine PUBLIC -static-libstdc++)
endif()

if (NEED_GLM)
	message(STATUS "Linking bitengine with glm")
	target_link_libraries(bitengine PUBLIC glm)
endif()
if (NEED_NLOHMANN_JSON)
	message(STATUS "Linking bitengine with nlohmann_json")
	target_link_libraries(bitengine PUBLIC nlohmann_json)
endif()

target_include_directories(bitengine PUBLIC $<BUILD_INTERFACE:${BITENGINE_INCLUDE_DIR}>
											$<INSTALL_INTERFACE:include/>)

install(TARGETS bitengine DESTINATION lib EXPORT bitengineTargets)
