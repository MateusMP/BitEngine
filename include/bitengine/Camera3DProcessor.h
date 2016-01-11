#pragma once

#include <unordered_set>

#include "Camera3DComponent.h"
#include "ComponentProcessor.h"

#include "Transform3DProcessor.h"
#include "EntitySystem.h"


namespace BitEngine{

	class Camera3DProcessor : public ComponentProcessor, public ComponentHolder
	{
	public:
		Camera3DProcessor();
		
		/// Holder
		Component* getComponent(ComponentHandle component) override;

		/// Processor
		bool Init(BaseEntitySystem* es) override;
		void Stop() override;
		void Process();

		void OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component) override;
		void OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component) override;

	private:

		ComponentHandle AllocComponent() override;
		void DeallocComponent(ComponentHandle component) override;

		const std::vector<ComponentHandle>& getComponents() const override;

		//
		void recalculateViewMatrix(Camera3DComponent* c, const glm::mat4& t);

	private:
		ComponentCollection<Camera3DComponent> components;

		// Processor
		struct Entry {
			Entry(EntityHandle ent, ComponentHandle cam, ComponentHandle transf) 
				: entity(ent), camera3d(cam), transform3d(transf) {}
			EntityHandle entity;
			ComponentHandle camera3d;
			ComponentHandle transform3d;
		};

		// Hold the ComponentType for all components this processor cares
		ComponentType Transform3DType;
		ComponentType Camera3DType;

		// Hold the ComponentHolders for all ComponentTypes cared by this processor
		Camera3DProcessor* holderCamera;
		Transform3DProcessor* holderTransform;

		// All entities that currently have the required components to be processed by this processor
		std::vector<Entry> processEntries;
		std::unordered_set<EntityHandle> invalidatedEntries;

		// The BaseEntitySystem this processor is working for
		BaseEntitySystem* baseES;
	};

}