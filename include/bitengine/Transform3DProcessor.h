#pragma once

#include "ComponentProcessor.h"
#include "Transform3DComponent.h"

namespace BitEngine{

	// TODO: Fix parents on childs when a parent Transform is destroyed
	class Transform3DProcessor : public ComponentProcessor, public ComponentHolder
	{
	public:
		Transform3DProcessor();
		~Transform3DProcessor();

		Component* getComponent(ComponentHandle component) override;

		bool Init(BaseEntitySystem* es) override;
		void Stop() override;
		void Process();

		void OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component) override
		{}
		void OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component) override
		{}

	private:

		ComponentHandle AllocComponent() override;
		void DeallocComponent(ComponentHandle component) override;

		const std::vector<ComponentHandle>& getComponents() const override;

	private:
		void calculateModelMatrix(Transform3DComponent* comp, glm::mat4& mat);
		int calculateParentRootDistance(Transform3DComponent* t);

		ComponentCollection<Transform3DComponent> components;
	};

}