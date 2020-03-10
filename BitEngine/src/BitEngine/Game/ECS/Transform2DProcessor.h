#pragma once

#include "BitEngine/Game/ECS/ComponentProcessor.h"
#include "BitEngine/Game/ECS/Transform2DComponent.h"

#include "BitEngine/Common/MathUtils.h"

namespace BitEngine{

	/**
	 * TODO: Fix parents on childs when a parent Transform is destroyed
	 * Basic Tranform 2D Implementation
	 * Uses Transform2DComponents
	 * Can create hierarchies and generate local and global transform matrices
	 **/
	class BE_API Transform2DProcessor : public ComponentProcessor,
        public Messenger< MsgComponentCreated<Transform2DComponent>>::ScopedSubscription,
        public Messenger< MsgComponentDestroyed<Transform2DComponent>>::ScopedSubscription
	{
		public:
			Transform2DProcessor(EntitySystem* es);
			~Transform2DProcessor();

			// Recalculate global transforms
			void Process();

			// Processor input
			inline void setParentOf(ComponentRef<Transform2DComponent>& a, ComponentRef<Transform2DComponent>& parent){
				setParentOf(ComponentProcessor::getComponentHandle(a), ComponentProcessor::getComponentHandle(parent));
			}
			
			// Calculate the rotation angle for given matrix
			static inline glm::mat3::value_type calculateAngleFromTransformMatrix(const glm::mat3& mat)
			{
				return atan2(mat[1][0], mat[1][1]);
			}

			// Calculates the position for given matrix
			static inline glm::vec2 calculatePositionFromTransformMatrix(const glm::mat3& mat)
			{
				return glm::vec2(mat[0][2], mat[1][2]);
			}

			// Calculates the X,Y scale applied on given matrix
			static inline glm::vec2 calculateScaleFromTransformMatrix(const glm::mat3& mat)
			{
				const auto a = mat[0][0];
				const auto b = mat[0][1];
				const auto c = mat[1][0];
				const auto d = mat[1][1];
				return glm::vec2(BitEngine::sign(a) * sqrt(a*a + b*b),
								 BitEngine::sign(d) * sqrt(c*c + d*d) );
			}

			// Message handling
			void onMessage(const MsgComponentCreated<Transform2DComponent>& msg);
			void onMessage(const MsgComponentDestroyed<Transform2DComponent>& msg);

		protected:
			// Calculate the local model matrix for given Transform2DComponent
			// This is exposed to simplify implementations of customs Transform2DProcessors
			static void CalculateLocalModelMatrix(const Transform2DComponent& component, glm::mat3& mat);


		private: // Functions
			void setParentOf(ComponentHandle a, ComponentHandle parent);

			// Processor
			struct Hierarchy
			{
				public:
					Hierarchy()
							: dirty(true)
					{
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

					void addChild(ComponentHandle a)
					{
						childs.push_back(a);
					}

					// Members
					//ComponentHandle self;
					ComponentHandle parent;
					bool dirty;
					std::vector<ComponentHandle> childs;
			};

			// TODO: Make iterative version
			void recalcGlobalTransform(ComponentHolder<SceneTransform2DComponent>* holder, SceneTransform2DComponent& scene, Hierarchy &t);

		private: // Attributes

			// Processor
			std::vector<Hierarchy> hierarchy;
	};

}
