#pragma once

#include <set>
#include <vector>
#include <functional>
#include <map>

#include "Core/Message.h"

namespace BitEngine {
	
	// Used to send messages @See Message<T>
	// There is one Messenger for each GameEngine.
	class Messenger
	{
		public:
			typedef std::function<void(const BaseMessage&)> Callback;

			Messenger();
			void Update();

			void RegisterListener(u32 msgType, void* handler, Callback call);
			
			template<typename MessageType>
			void RegisterListener(void* handler, Callback call) {
				RegisterListener(MessageType::MessageType(), handler, call);
			}

			void UnregisterListener(void* handler);

			void SendMessage(const BaseMessage& msg);

			template<typename T>
			void SendDelayedMessage(const T& msg) {
				copyMessage(&msg, sizeof(T));
			}

		private:
			void copyMessage(const BaseMessage* msg, u32 size);

			struct Call{
				void* obj;
				Callback callback;
			};

			std::multimap<u32, Call> m_callbacks;

			std::vector<char> m_messageQueue;
			std::set<BaseMessage*> m_QueuedMessages;
	};

	// This class is a Messenger End Point
	// Any class that wants to receive messages should come from this
	class MessengerEndpoint
	{
		public:
			MessengerEndpoint() 
				: m_messenger(nullptr)
			{}

			Messenger* getMessenger() { return m_messenger; }
			void setMessenger(Messenger* m) { m_messenger = m; }

		private:
			Messenger* m_messenger;
	};

}
