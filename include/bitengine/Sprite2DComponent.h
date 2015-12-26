#pragma once

#include "Component.h"
#include "SpriteManager.h"

#include "Sprite.h"

namespace BitEngine{

class Sprite2DComponent : public Component
{
public:

	Sprite2DComponent()
		: depth(0), sprite(0)
	{}

	int depth;
	SpriteHandle sprite;

};

}