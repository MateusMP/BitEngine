#pragma once

#include <set>
#include <vector>
#include <functional>
#include <map>
#include <memory>
#include <typeindex>

#include "bitengine/Core/Message.h"
#include "bitengine/Core/GameEngine.h"

namespace BitEngine {


	class Messenger : public EnginePiece
	{
		public:
		Messenger(GameEngine* ge) : EnginePiece(ge) {}

		void dispatchEnqueued()
		{
			for (auto& it : dispatchers)
			{
				it.second->dispatchAllEnqueued();
			}
		}

		template<typename MessageType, typename Handler>
		void registerListener(Handler* handler)
		{
			auto addr = new Messaging::MessageHandleBridge<MessageType, Handler>(handler);
			//printf("Registering: %p with bridge %p\n", handler, addr);
			getDispatcher<MessageType>().registerListener(handler, addr);
		}

		template<typename MessageType, typename Handler>
		void unregisterListener(Handler* handler)
		{
			getDispatcher<MessageType>().unregisterListener(handler);
		}

		// Instant dispatch
		template<typename MessageType>
		void dispatch(const MessageType& msg)
		{
			getDispatcher<MessageType>().typedDispatch(msg);
		}

		// Instant dispatch
		template<typename MessageType>
		void delayedDispatch(const MessageType& msg)
		{
			getDispatcher<MessageType>().typedEnqueue(msg);
		}

		private:

		template<typename MessageType>
		Messaging::TypedMessageDispatcher<MessageType>& getDispatcher()
		{
			auto typeIndex = std::type_index(typeid(MessageType));
			auto it = dispatchers.find(typeIndex);
			if (it == dispatchers.end())
			{
				Messaging::TypedMessageDispatcher<MessageType> *value = new Messaging::TypedMessageDispatcher<MessageType>();
				dispatchers.emplace(typeIndex, value);
				return *value;
			}
			else
			{
				return (Messaging::TypedMessageDispatcher<MessageType>&) *((it->second).get());
			}
		}

		std::map<std::type_index, std::unique_ptr<Messaging::MessageDispatcher>> dispatchers;
	};

	namespace Messaging
	{
		// This class is a Messenger End Point
		// Any class that wants to receive messages should come from this
		class MessengerEndpoint
		{
			public:
				MessengerEndpoint(MessengerEndpoint* m)
					: m_messenger(m->m_messenger)
				{}
				MessengerEndpoint(Messenger* m)
					: m_messenger(m)
				{}

				Messenger* getMessenger() { return m_messenger; }

			private:
				Messenger* m_messenger;
		};

	}
}
