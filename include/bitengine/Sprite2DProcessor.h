#pragma once

#include <unordered_map>

#include "Sprite2DComponent.h"
#include "Camera2DProcessor.h"

namespace BitEngine{

class Sprite2DProcessor : public ComponentHolder
{
	public:
		Sprite2DProcessor();
		~Sprite2DProcessor();

	private:
		ComponentHandle AllocComponent() override;
		void DeallocComponent(ComponentHandle component) override;

		Component* getComponent(ComponentHandle hdl) override;

		const std::vector<ComponentHandle>& getComponents() const override;

	private:
		ComponentCollection<Sprite2DComponent> components;
};

}