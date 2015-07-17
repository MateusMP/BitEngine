#pragma once

#include "Component.h"
#include "EntitySystem.h"

#include "Sprite.h"

namespace BitEngine{


class Sprite2DComponent : public Component
{
public:
	enum class SORT_TYPE : char{
		NOONE,
		BY_TEXTURE_ONLY,
		BY_DEPTH_ONLY,
		BY_INVDEPTH_ONLY,
		BY_DEPTH_TEXTURE,
		BY_TEXTURE_DEPTH,

		TOTAL
	};

	Sprite2DComponent()
		: x(0), y(0), depth(0), width(8), height(8), sortMode(SORT_TYPE::BY_DEPTH_TEXTURE)
	{}
	int x;
	int y;
	int depth;
	int width;
	int height;
	Sprite sprite;
	SORT_TYPE sortMode;

	static ComponentType getComponentType(){
		return COMPONENT_TYPE_SPRITE2D;
	}
};

}