#pragma once

#include <vector>
#include "Component.h"
#include "ComponentsType.h"

#include "EntitySystem.h"

namespace BitEngine{


	class CollisionLogic
	{
		virtual void OnCollision() = 0;
	};

	class EntityHolderComponent : public Component{
		public:
			EntityHolderComponent(EntityHandle ent, EntitySystem* sys)
				: m_myEntity(ent), e_sys(sys) {
				printf("%p EntityHolderComponent(%u, %p)\n", this, ent, sys);
			}

			EntityHandle m_myEntity;
			EntitySystem* e_sys;
	};

	class GameLogic
	{
		public:
			enum RunEvents : uint16 {
				EFrameStart = 1,
				EFrameMiddle = 2,
				EFrameEnd = 4,

				EALL = EFrameStart | EFrameMiddle | EFrameEnd
			};

			virtual RunEvents getRunEvents() = 0;

			virtual bool Init() = 0;

			virtual void FrameStart() = 0;
			virtual void FrameMiddle() = 0;
			virtual void FrameEnd() = 0;

			virtual void End() = 0;

			template<typename CompClass>
			bool getComponent(ComponentRef<CompClass>& ref){
				return e_sys->getComponentRef(m_myEntity, ref);
			}

		private:
			friend class GameLogicComponent;
			EntityHandle m_myEntity;
			EntitySystem* e_sys;
	};

	class GameLogicComponent : public EntityHolderComponent
	{
	public:
		GameLogicComponent()
			: EntityHolderComponent(0, nullptr) {}
		GameLogicComponent(EntityHandle ent, EntitySystem* sys)
			: EntityHolderComponent(ent, sys) {}

		void addLogicPiece(GameLogic* logic){
			m_gamelogics.emplace_back(logic);
			logic->m_myEntity = m_myEntity;
			logic->e_sys = e_sys;
		}

		static ComponentType getComponentType(){
			return COMPONENT_TYPE_GAMELOGIC;
		}

	private:
		friend class GameLogicProcessor;

		std::vector<GameLogic*> m_gamelogics;
	};

}