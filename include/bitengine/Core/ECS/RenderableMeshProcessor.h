#pragma once

#include "Core/ECS/RenderableMeshComponent.h"

#include "Core/ECS/ComponentProcessor.h"
#include "Core/ECS/EntitySystem.h"


namespace BitEngine
{

	class RenderableMeshProcessor : public ComponentProcessor, public ComponentHolder
	{
	public:
		RenderableMeshProcessor();

		Component* getComponent(ComponentHandle component) override;

		bool Init(BaseEntitySystem* es) override;
		void Stop() override;

		void OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component)
		{}
		void OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component)
		{}

	private: // Functions
		ComponentHandle AllocComponent() override;
		void DeallocComponent(ComponentHandle component) override;

		const std::vector<ComponentHandle>& getComponents() const override;

	private: // Variables
		ComponentCollection<RenderableMeshComponent> components;
	};


}