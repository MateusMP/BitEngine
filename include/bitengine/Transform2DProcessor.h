#pragma once

#include "ComponentProcessor.h"
#include "Transform2DComponent.h"

namespace BitEngine{

	// TODO: Fix parents on childs when a parent Transform is destroyed
	class Transform2DProcessor : public ComponentProcessor, public ComponentHolder
	{
		public:
			Transform2DProcessor();
			~Transform2DProcessor();

			/// Holder
			Component* getComponent(ComponentHandle component) override;

			/// Processor
			bool Init(BaseEntitySystem* es) override;
			void Stop() override;
			void Process();

			void OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component) override {}
			void OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component) override {}

		private:

			/// Holder
			ComponentHandle AllocComponent() override;
			void DeallocComponent(ComponentHandle component) override;

			const std::vector<ComponentHandle>& getComponents() const override;

		private:

			/// Processor
			void calculateModelMatrix(Transform2DComponent* comp, glm::mat3& mat);
			int calculateParentRootDistance(Transform2DComponent* t);

			ComponentCollection<Transform2DComponent> components;
	};

}