#pragma once

#include "Core/ECS/Component.h"
#include "Core/SpriteManager.h"

#include "Core/Sprite.h"

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