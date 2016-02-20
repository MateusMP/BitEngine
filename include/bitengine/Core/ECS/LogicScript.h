#pragma once

#include <vector>

#include "Common/MathUtils.h"

#include "Core/ECS/Component.h"
#include "Core/ECS/EntitySystem.h"

namespace BitEngine{


	class CollisionLogic
	{
		virtual void OnCollision() = 0;
	};

	class GameLogic : public MessengerEndpoint
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
			ComponentRef<CompClass> getComponent() {
				return e_sys->getComponentRef<CompClass>(m_myEntity);
			}

		private:
			friend class GameLogicComponent;
			EntityHandle m_myEntity;
			EntitySystem* e_sys;
	};

	class GameLogicComponent : public Component<GameLogicComponent>
	{
        public:
	        GameLogicComponent(){}
                //: EntityHolderComponent(0, nullptr) {}
            GameLogicComponent(EntityHandle ent){}
                //: EntityHolderComponent(ent, sys) {}

            void addLogicPiece(GameLogic* logic){
                m_gamelogics.emplace_back(logic);
                logic->m_myEntity = getEntity();
                logic->e_sys = e_sys;
            }

			// std::vector<GameLogic*>& getLogics() { return m_gamelogics; }

        private:
            friend class GameLogicProcessor;
            EntitySystem* e_sys;
            std::vector<GameLogic*> m_gamelogics;
	};

}
