#pragma once

#include "Core/ECS/ComponentProcessor.h"
#include "Core/ECS/Transform2DComponent.h"

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

			void OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component) override
			{
				if (localTransform.size() <= component) {
					localTransform.resize(component + 1);
					globalTransform.resize(component + 1);
					hierarchy.resize(component + 1);
					hierarchy[component].self = component;
				}
			}

			void OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component) override
			{
				// Childs lost their parent. Let them to the previous parent root.
				for (ComponentHandle c : hierarchy[component].childs)
					setParentOf(c, hierarchy[component].parent);
			}

			// Processor input
			void setParentOf(ComponentHandle a, ComponentHandle parent)
			{
				const ComponentHandle prevParent = hierarchy[a].parent;
				if (prevParent == parent)
					return;

				// Remove child from previous parent
				hierarchy[prevParent].removeChild(a);

				// Set parent for given component
				hierarchy[a].parent = parent;
				hierarchy[a].dirty = true;

				// Add child for new parent
				hierarchy[parent].addChild(a);
			}

			// Processor result outputs
			const std::vector<glm::mat3>& getGlobalTransforms() const {
				return globalTransform;
			}
			const glm::mat3& getGlobalTransformFor(ComponentHandle hdl) const {
				return globalTransform[hdl];
			}

		private: // Functions
				 // Holder
			ComponentHandle AllocComponent() override;
			void DeallocComponent(ComponentHandle component) override;

			const std::vector<ComponentHandle>& getComponents() const override;

			// Processor
			static void CalculateLocalModelMatrix(const Transform2DComponent* component, glm::mat3& mat);

			struct Hierarchy {
				Hierarchy() {
					self = 0;
					parent = 0;
				}

				void removeChild(ComponentHandle a)
				{
					for (ComponentHandle& h : childs)
					{
						if (h == a)
						{
							h = childs.back();
							childs.pop_back();
							break;
						}
					}
				}

				void addChild(ComponentHandle a) {
					childs.push_back(a);
				}

				// Members
				ComponentHandle self;
				ComponentHandle parent;
				bool dirty;
				std::vector<ComponentHandle> childs;
			};

			// TODO: Make iterative version
			void RecalcGlobal(Hierarchy &t);

		private: // Attributes

				 // Holder
			ComponentCollection<Transform2DComponent> components;


			// Processor
			std::vector<Hierarchy> hierarchy;
			std::vector<glm::mat3> localTransform; // used only to calculate globalTransform
			std::vector<glm::mat3> globalTransform; // information used by external systems
	};

}
