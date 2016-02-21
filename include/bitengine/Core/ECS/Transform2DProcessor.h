#pragma once

#include "Core/ECS/ComponentProcessor.h"
#include "Core/ECS/Transform2DComponent.h"

namespace BitEngine{

	// TODO: Fix parents on childs when a parent Transform is destroyed
	class Transform2DProcessor : public ComponentProcessor
	{
		public:
			Transform2DProcessor();
			~Transform2DProcessor();

			/// Processor
			bool Init() override;
			void Stop() override;
			void Process();

			void onTransform2DComponentCreated(const BaseMessage& msg_);
			void onTransform2DComponentDestroyed(const BaseMessage& msg_);

			// Processor input
			inline void setParentOf(ComponentRef<Transform2DComponent>& a, ComponentRef<Transform2DComponent>& parent){
				setParentOf(ComponentProcessor::getComponentHandle(a), ComponentProcessor::getComponentHandle(parent));
			}

			// Processor result outputs
			inline const std::vector<glm::mat3>& getGlobalTransforms() const {
				return globalTransform;
			}
			inline const glm::mat3& getGlobalTransformFor(ComponentHandle hdl) const {
				return globalTransform[hdl];
			}

		private: // Functions
			void setParentOf(ComponentHandle a, ComponentHandle parent);

			// Processor
			static void CalculateLocalModelMatrix(const Transform2DComponent& component, glm::mat3& mat);

			struct Hierarchy {
				Hierarchy() {
					//self = 0;
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
				//ComponentHandle self;
				ComponentHandle parent;
				bool dirty;
				std::vector<ComponentHandle> childs;
			};

			// TODO: Make iterative version
			void recalcGlobalTransform(ComponentHandle handle, Hierarchy &t);

		private: // Attributes

			// Processor
			std::vector<Hierarchy> hierarchy;
			std::vector<glm::mat3> localTransform; // used only to calculate globalTransform
			std::vector<glm::mat3> globalTransform; // information used by external systems
	};

}
