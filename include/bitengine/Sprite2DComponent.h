#pragma once

#include "Component.h"
#include "ComponentsType.h"
#include "SpriteManager.h"

#include "Sprite.h"

namespace BitEngine{

class Sprite2DComponent : public Component
{
public:

	static ComponentType getComponentType(){
		return COMPONENT_TYPE_SPRITE2D;
	}

	Sprite2DComponent()
		: depth(0), sortMode(SpriteSortType::BY_DEPTH_TEXTURE), sprite(0)
	{}

	int depth;
	SpriteSortType sortMode;
	SpriteHandle sprite;

};

}