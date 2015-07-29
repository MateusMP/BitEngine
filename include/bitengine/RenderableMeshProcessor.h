#pragma once

#include "ComponentProcessor.h"

#include "EntitySystem.h"

#include "RenderableMeshComponent.h"

namespace BitEngine
{

	class RenderableMeshProcessor : public ComponentHolderProcessor
	{
	public:
		RenderableMeshProcessor();

		Component* getComponent(ComponentHandle component) override;

	private: // Functions
		bool Init() override;
		void FrameStart() override {};
		void FrameMiddle() override {};
		void FrameEnd() override {};

		ComponentHandle CreateComponent(EntityHandle entity) override;
		void DestroyComponent(ComponentHandle component) override;

		const std::vector<ComponentHandle>& getComponents() const override;

	private: // Variables
		ComponentCollection<RenderableMeshComponent> components;
	};


}