#pragma once

#include "Core/ECS/Camera2DComponent.h"
#include "Core/ECS/ComponentProcessor.h"

#include "Core/ECS/Transform2DProcessor.h"
#include "Core/ECS/EntitySystem.h"

#include "Common/VectorBool.h"

namespace BitEngine{

class Camera2DProcessor : public ComponentProcessor, public ComponentHolder
{
	public: // Methods
		Camera2DProcessor();

		/// Holder
		Component* getComponent(ComponentHandle component) override;

		/// Processor
		bool Init(BaseEntitySystem* es) override;
		void Stop() override;
		void Process();

		void OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component) override;
		void OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component) override;
		
	private: // Methods

		/// Component Holder
		ComponentHandle AllocComponent() override;
		void DeallocComponent(ComponentHandle component) override;
		
		const std::vector<ComponentHandle>& getComponents() const override;

		/// Processor
		void recalculateMatrix(Camera2DComponent* c, const Transform2DComponent* t);
		
	private: // Member variables

		/// ComponentHolder part
		// The collection of components
		ComponentCollection<Camera2DComponent> components;

		/// Component processor

		// Hold the ComponentType for all components this processor cares
		ComponentType Transform2DType;
		ComponentType Camera2DType;

		// Hold the ComponentHolders for all ComponentTypes cared by this processor
		ComponentHolder* holderCamera;
		ComponentHolder* holderTransform;

		// All entities that currently have the required components to be processed by this processor
		std::vector<EntityHandle> processEntities;

		// The BaseEntitySystem this processor is working for
		BaseEntitySystem* baseES;
};

}