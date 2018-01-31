#pragma once

#include <set>
#include <vector>
#include <functional>
#include <map>
#include <memory>
#include <typeindex>
#include <cstring>

#include "bitengine/Common/TypeDefinition.h"
#include "bitengine/Core/GameEngine.h"

namespace BitEngine {
	
	class Messenger
	{
		private:		

		template <class EventType>
		using call_type = std::function<void(const EventType&)>;
		template <class EventType, class Handler>
		using member_func_t = void (Handler::*)(const EventType&);

		struct MessageType;
		struct EnqueuedEvent;

		typedef const void* Message;

		// Subscription Handle
		typedef u32 SubsHandle;

		public:

		Messenger() : handles(0) {
			m_subscribers.reserve(128);
		}

		template<typename EventType, typename Handler>
		SubsHandle subscribe(member_func_t<EventType, Handler> func, Handler* handler) {
			return _subscribe<EventType, Handler>(func, handler);
		}

		/**
		* Subscriber a handler to an EventType.
		* The handler method should follow the signature: void (const EventType&)
		*/
		template<typename EventType>
		SubsHandle subscribe(call_type<EventType> callable) {

			u32 type = MessageType::type<EventType>();
			if (m_subscribers.size() <= type) {
				m_subscribers.resize(type + 1);
			}

			//callable(EventType{2});
			SubsHandle handle = ++handles;
			m_subscribers[type].emplace_back(CallbackWrapper<EventType>(callable), handle);
			return handle;
		}

		template<typename EventType>
		void unsubscribe(SubsHandle handle) {

			u32 type = MessageType::type<EventType>();
			unsubscribe(handle, type);
		}

		void unsubscribe(SubsHandle handle, u32 type) {

			if (m_subscribers.size() <= type) {
				m_subscribers.resize(type + 1);
			}

			auto& listeners = m_subscribers[type];
			for (auto it = listeners.begin(); it != listeners.end(); ++it) {
				if (it->handle == handle) {
					listeners.erase(it);
					break;
				}
			}
		}

		/**
		* Emit a message to all subscribers. Handler are called immediately.
		*/
		template <typename EventType>
		void emit(const EventType& event) {

			u32 type = MessageType::type<EventType>();
			if (m_subscribers.size() <= type) {
				return;
			}

			Message eventWrapper(&event);
			for (auto& receiver : m_subscribers[type]) {
				receiver.callback(eventWrapper);
			}
		}

		/**
		* Emit a message that is deferred until a dispatch() call.
		*/
		template <typename EventType>
		void enqueue(const EventType& event)
		{
			u32 type = MessageType::type<EventType>();

			u32 idx = m_enqueuedEventData.size();
			m_enqueuedEventData.resize(idx + sizeof(event));
			m_enqueuedEvents.push_back({ type, idx });

			memcpy(m_enqueuedEventData.data() + idx, &event, sizeof(event));
		}

		/**
		* Dispatch all enqueued messages in the order that they were enqueued.
		*/
		void dispatch() {
			m_enqueuedEventDataBuffer.swap(m_enqueuedEventData);
			m_enqueuedEventsBuffer.swap(m_enqueuedEvents);

			for (const EnqueuedEvent& ev : m_enqueuedEventsBuffer) {
				if (m_subscribers.size() <= ev.type) {
					continue;
				}

				const void* b = (void*)(m_enqueuedEventDataBuffer.data() + ev.idx);
				Message msg(b);
				for (auto& receiver : m_subscribers[ev.type]) {
					receiver.callback(msg);
				}
			}
			m_enqueuedEventsBuffer.clear();
			m_enqueuedEventDataBuffer.clear();
		}

		private:

		template<typename EventType, typename Handler>
		SubsHandle _subscribe(member_func_t<EventType, Handler> func, Handler* handler) {
			return subscribe<EventType>([handler, func](const EventType& ev) -> void { (handler->*func)(ev); });
		}


		struct callback_handle {
			callback_handle(call_type<Message> c, SubsHandle h) : callback(c), handle(h) {
			}
			call_type<Message> callback;
			SubsHandle handle;
		};

		std::vector < std::vector< callback_handle >> m_subscribers;
		std::vector<char> m_enqueuedEventData;
		std::vector<char> m_enqueuedEventDataBuffer;
		std::vector<EnqueuedEvent> m_enqueuedEvents;
		std::vector<EnqueuedEvent> m_enqueuedEventsBuffer;
		u32 handles;

		template <typename EventType>
		struct CallbackWrapper
		{
			CallbackWrapper(const call_type<EventType>& callable) : m_callable(callable) {}

			void operator() (const Message& msg) {
				m_callable(*static_cast<const EventType*>(msg));
			}

			call_type<EventType> m_callable;
		};


		struct MessageType {
			static u32 getNextType();

			template<typename EventType>
			static u32 type()
			{
				static u32 t_type = MessageType::getNextType();
				return t_type;
			}
		};

		struct EnqueuedEvent {
			u32 type;
			u32 idx;
		};

	public:

		template<typename EventType>
		static u32 TypeFor() {
			return MessageType::type<EventType>();
		}

		template<typename EventType>
		class ScopedSubscription {
			public:
			template<typename Handler>
			ScopedSubscription(Messenger& msg, member_func_t<EventType, Handler> func, Handler* handler)
				: messenger(msg) {
				handle = messenger.subscribe(func, handler);
			}
			ScopedSubscription(Messenger& msg, call_type<EventType> call)
				: messenger(msg) {
				handle = messenger.subscribe(call);
			}
			ScopedSubscription(Messenger& msg, u32 h)
				: messenger(msg), handle(h) {
			}
			~ScopedSubscription() {
				messenger.unsubscribe<EventType>(handle);
			}
			u32 handle;
			Messenger& messenger;
		};
	};

	// This class is a Messenger End Point
	// Any class that needs to receive messages is recommended to extend this
	// to avoid complications in subscriptions/unsubscriptions
	// Usually, the derivated class will call the subscribe method in the constructor
	// and forget, since it'll be automagically unsubscribed on destruction of the object.
	class MessengerEndpoint
	{
		public:
			MessengerEndpoint(Messenger* m)
				: m_messenger(m)
			{}

			virtual ~MessengerEndpoint() {
				unsubscribeAll();
			}

			Messenger* getMessenger() { return m_messenger; }

		protected:
			void unsubscribeAll() {
				for (Subscription s : m_eventSubscriptions) {
					m_messenger->unsubscribe(s.handle, s.type);
				}
				m_eventSubscriptions.clear();
			}

			template<typename EventType, typename Handler>
			void subscribe(void (Handler::*func)(const EventType&), Handler* handler) {
				u32 type = Messenger::TypeFor<EventType>();
				u32 handle = m_messenger->subscribe<EventType, Handler>(func, handler);
				m_eventSubscriptions.emplace_back(Subscription{handle, type});
			}

		private:
			struct Subscription {
				u32 handle;
				u32 type;
			};

			std::vector<Subscription> m_eventSubscriptions;
			Messenger* m_messenger;


	};
}
