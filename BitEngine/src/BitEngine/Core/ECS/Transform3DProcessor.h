#pragma once

#include "bitengine/Core/ECS/ComponentProcessor.h"
#include "bitengine/Core/ECS/Transform3DComponent.h"

namespace BitEngine{

	/**
	 * TODO: Fix parents on childs when a parent Transform is destroyed
	 */
	class Transform3DProcessor : public ComponentProcessor
	{
	public:
		Transform3DProcessor(Messenger* m);
		~Transform3DProcessor();

		// Processor
		bool Init() override;
		void Stop() override;
		void Process();

		// Processor result outputs
		const std::vector<glm::mat4>& getGlobalTransforms() const {
			return globalTransform;
		}
		const glm::mat4& getGlobalTransformFor(ComponentHandle hdl) const {
			return globalTransform[hdl];
		}

		// Make 'a' become child of 'parent
		void setParentOf(ComponentRef<Transform3DComponent>& a, ComponentRef<Transform3DComponent>& parent)
		{
			setParentOf(ComponentProcessor::getComponentHandle(a), ComponentProcessor::getComponentHandle(parent));
		}

		// Message handlers
		void onMessage(const MsgComponentCreated<Transform3DComponent>& msg);
		void onMessage(const MsgComponentDestroyed<Transform3DComponent>& msg);

	private: // Functions
        struct Hierarchy;
		static void CalculateLocalModelMatrix(const Transform3DComponent& component, glm::mat4& mat);


		// TODO: Make iterative version
		void recalcGlobalTransform(ComponentHandle handle, Hierarchy &t);

		void setParentOf(ComponentHandle a, ComponentHandle parent);

		struct Hierarchy {
			Hierarchy() : dirty(true){
				parent = 0;
			}

			void removeChild(ComponentHandle a) {
				for (ComponentHandle& h : childs)
				{
					if (h == a) {
						h = childs.back();
						childs.pop_back();
						break;
					}
				}
			}

			void addChild(ComponentHandle a) {
				childs.push_back(a);
			}

			ComponentHandle parent;
			bool dirty;
			std::vector<ComponentHandle> childs;
		};

	private: // Attributes

		// Processor
		std::vector<Hierarchy> hierarchy;
		std::vector<glm::mat4> localTransform; // used only to calculate globalTransform
		std::vector<glm::mat4> globalTransform; // information used by external systems

	};

}
