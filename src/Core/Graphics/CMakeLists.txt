set(SRCS
    Sprite2D.cpp
)

add_library(be_graphics OBJECT ${SRCS})
target_include_directories(be_graphics PUBLIC ${ALL_BE_INCLUDES})