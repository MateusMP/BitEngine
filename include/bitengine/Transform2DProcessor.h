#pragma once

#include "ComponentProcessor.h"
#include "Transform2DComponent.h"

namespace BitEngine{


	class Transform2DProcessor : public ComponentHolderProcessor
	{
		public:
			Transform2DProcessor();
			~Transform2DProcessor();

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
			void calculateModelMatrix(Transform2DComponent* comp, glm::mat3& mat);
			int calculateParentRootDistance(Transform2DComponent* t);

			ComponentCollection<Transform2DComponent> components;
	};

}