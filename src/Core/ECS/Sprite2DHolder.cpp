#include "Core/ECS/Sprite2DHolder.h"

#include <algorithm>

namespace BitEngine{


Sprite2DHolder::Sprite2DHolder()
{
}

Sprite2DHolder::~Sprite2DHolder()
{
}

const std::vector<ComponentHandle>& Sprite2DHolder::getComponents() const
{
	return components.getValidComponents();
}

ComponentHandle Sprite2DHolder::AllocComponent()
{
	return components.newComponent();
}

void Sprite2DHolder::DeallocComponent(ComponentHandle component)
{
	components.removeComponent(component);
}

Component* Sprite2DHolder::getComponent(ComponentHandle hdl)
{
	return components.getComponent(hdl);
}


}