#pragma once

#include "Component.h"
#include "ComponentsType.h"
#include "SpriteManager.h"

#include "Sprite.h"

namespace BitEngine{


class Sprite2DComponent : public Component
{
public:
	enum class SORT_TYPE : char{
		NOONE,
		BY_DEPTH_TEXTURE,

		TOTAL
	};

	static ComponentType getComponentType(){
		return COMPONENT_TYPE_SPRITE2D;
	}

	Sprite2DComponent()
		: depth(0), sortMode(SORT_TYPE::BY_DEPTH_TEXTURE), sprite(0)
	{}

	int depth;
	SORT_TYPE sortMode;
	SpriteHandle sprite;

};

}