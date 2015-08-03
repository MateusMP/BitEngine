#pragma once

#include "Camera3DComponent.h"
#include "ComponentProcessor.h"

#include "Transform3DProcessor.h"
#include "EntitySystem.h"

namespace BitEngine{

	class Camera3DProcessor : public ComponentHolderProcessor
	{
	public:
		Camera3DProcessor(EntitySystem* es);
		
		Component* getComponent(ComponentHandle component) override;

	private:
		bool Init() override;
		void FrameStart() override {};
		void FrameMiddle() override {};
		void FrameEnd() override;

		ComponentHandle CreateComponent(EntityHandle entity) override;
		void DestroyComponent(ComponentHandle component) override;

		const std::vector<ComponentHandle>& getComponents() const override;

		//
		void recalculateViewMatrix(Camera3DComponent* c, const Transform3DComponent* t);

	private:
		EntitySystem* m_entitySys;

		ComponentCollection<Camera3DComponent> components;
	};

}