#pragma once

#include "ComponentProcessor.h"
#include "Transform3DComponent.h"

namespace BitEngine{


	class Transform3DProcessor : public ComponentHolderProcessor
	{
	public:
		Transform3DProcessor();
		~Transform3DProcessor();

		Component* getComponent(ComponentHandle component) override;

	private:
		bool Init() override;
		void FrameStart() override {};
		void FrameMiddle() override {};
		void FrameEnd() override;

		ComponentHandle CreateComponent(EntityHandle entity) override;
		void DestroyComponent(ComponentHandle component) override;


		const std::vector<ComponentHandle>& getComponents() const override;

	private:
		void calculateModelMatrix(Transform3DComponent* comp, glm::mat4& mat);
		int calculateParentRootDistance(Transform3DComponent* t);

		ComponentCollection<Transform3DComponent> components;
	};

}