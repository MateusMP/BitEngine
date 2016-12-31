set(SRCS
    DevResourceLoader.cpp
	SpriteManager.cpp
)

add_library(be_resources OBJECT ${SRCS})
target_include_directories(be_resources PUBLIC ${ALL_BE_INCLUDES})