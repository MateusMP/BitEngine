#pragma once

#include <set>
#include <vector>
#include <functional>
#include <map>

#include "Core/Message.h"

namespace BitEngine {
	
	class Messenger
	{
		public:
			typedef std::function<void(const BaseMessage&)> Callback;

			Messenger();
			void Update();

			void RegisterListener(uint32 msgType, void* handler, Callback call);
			
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
			void copyMessage(const BaseMessage* msg, uint32 size);

			struct Call{
				void* obj;
				Callback callback;
			};

			std::multimap<uint32, Call> m_callbacks;

			std::vector<char> m_messageQueue;
			std::set<BaseMessage*> m_QueuedMessages;
	};


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
