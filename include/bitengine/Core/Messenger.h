#pragma once

#include <set>
#include <vector>
#include <functional>
#include <map>
#include <typeindex>

#include "Core/Message.h"

namespace BitEngine {

	namespace Messaging
	{
		class Messenger
		{
			public:
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
				TypedMessageDispatcher<MessageType>* dispatcher;
				auto typeIndex = std::type_index(typeid(MessageType));
				auto it = dispatchers.find(typeIndex);
				if (it == dispatchers.end())
				{
					dispatcher = new TypedMessageDispatcher<MessageType>();
					dispatchers.emplace(typeIndex, dispatcher);
				}
				else
				{
					dispatcher = static_cast<TypedMessageDispatcher<MessageType>*>(it->second);
				}

				auto addr = new MessageHandleBridge<MessageType, Handler>(handler);
				//printf("Registering: %p with bridge %p\n", handler, addr);
				dispatcher->registerListener(handler, addr);
			}

			template<typename MessageType, typename Handler>
			void unregisterListener(Handler* handler)
			{
				TypedMessageDispatcher<MessageType>* dispatcher;
				auto typeIndex = std::type_index(typeid(MessageType));
				auto it = dispatchers.find(typeIndex);
				if (it == dispatchers.end())
				{
					dispatcher = new TypedMessageDispatcher<MessageType>();
					dispatchers.emplace(typeIndex, dispatcher);
				}
				else
				{
					dispatcher = static_cast<TypedMessageDispatcher<MessageType>*>(it->second);
				}

				//printf("Registering: %p with bridge %p\n", handler, addr);
				dispatcher->unregisterListener(handler);
			}

			// Instant dispatch
			template<typename MessageType>
			void dispatch(const MessageType& msg)
			{
				auto typeIndex = std::type_index(typeid(MessageType));
				auto it = dispatchers.find(typeIndex);
				if (it == dispatchers.end())
				{
					//LOG(EngineLog, BE_LOG_WARNING) << "No dispatcher registered for this type.";
				}
				else
				{
					TypedMessageDispatcher<MessageType>* dispatcher = static_cast<TypedMessageDispatcher<MessageType>*>(it->second);
					dispatcher->typedDispatch(msg);
				}
			}

			// Instant dispatch
			template<typename MessageType>
			void delayedDispatch(const MessageType& msg)
			{
				TypedMessageDispatcher<MessageType>* dispatcher;
				auto typeIndex = std::type_index(typeid(MessageType));
				auto it = dispatchers.find(typeIndex);
				if (it == dispatchers.end())
				{
					dispatcher = new TypedMessageDispatcher<MessageType>();
					dispatchers.emplace(typeIndex, dispatcher);
				}
				else
				{
					dispatcher = static_cast<TypedMessageDispatcher<MessageType>*>(it->second);
				}

				dispatcher->typedEnqueue(msg);
			}

			private:
			std::map<std::type_index, MessageDispatcher*> dispatchers;
		};


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
