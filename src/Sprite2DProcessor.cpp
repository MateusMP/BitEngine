#include "Sprite2DProcessor.h"

#include <algorithm>

namespace BitEngine{


Sprite2DProcessor::Sprite2DProcessor()
{
}

Sprite2DProcessor::~Sprite2DProcessor()
{
}

const std::vector<ComponentHandle>& Sprite2DProcessor::getComponents() const
{
	return components.getValidComponents();
}

bool Sprite2DProcessor::Init()
{
	return true;
}

ComponentHandle Sprite2DProcessor::CreateComponent(EntityHandle entity)
{
	return components.newComponent();
}

void Sprite2DProcessor::DestroyComponent(ComponentHandle component)
{
	components.removeComponent(component);
}

Component* Sprite2DProcessor::getComponent(ComponentHandle hdl)
{
	return components.getComponent(hdl);
}


}