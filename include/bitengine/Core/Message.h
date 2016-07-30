#pragma once

#include "Common/TypeDefinition.h"

namespace BitEngine {
namespace Messaging {

	template<typename MessageType>
	class MessageHandler
	{
		public:
		virtual void handle(const MessageType& type) = 0;
	};

	template<typename MessageType, typename Handler>
	class MessageHandleBridge : public MessageHandler<MessageType>
	{
		public:
		MessageHandleBridge(Handler* h)
			: handler(h)
		{
			//printf("Bridge %p created for %p\n", this, handler);
		}
		void handle(const MessageType& type) override {
			//printf("Bridge %p handling %p\n", this, handler);
			handler->onMessage(type);
		}

		private:
		Handler* handler;
	};

	class MessageDispatcher
	{
		public:
		virtual ~MessageDispatcher() {}
		virtual void dispatch(const void *msg) = 0;
		virtual void enqueue(const void *msg) = 0;
		virtual void dispatchAllEnqueued() = 0;
	};

	template<typename MessageType>
	class TypedMessageDispatcher : public MessageDispatcher
	{
		public:
		void typedDispatch(const MessageType& message)
		{
			for (MessageHandler<MessageType>* handler : listeners) {
				handler->handle(message);
			}
		}

		void typedEnqueue(const MessageType& message)
		{
			queuedMessages.emplace_back(message);
		}

		void dispatchAllEnqueued() override
		{
			for (const MessageType& msg : queuedMessages) {
				typedDispatch(msg);
			}
			queuedMessages.clear();
		}

		void registerListener(void* realHandler, MessageHandler<MessageType>* msgHandler)
		{
			listeners.emplace(msgHandler);
			handlerToBridge.emplace(realHandler, msgHandler);
		}

		void unregisterListener(void* realHandler)
		{
			const auto& it = handlerToBridge.find(realHandler);
			if (it != handlerToBridge.end()) {
				delete it->second;
				listeners.erase(it->second);
				handlerToBridge.erase(it);
			}
		}

		private:
		void dispatch(const void *msg) override {
			typedDispatch(*static_cast<const MessageType*>(msg));
		}

		void enqueue(const void *msg) override {
			typedEnqueue(*static_cast<const MessageType*>(msg));
		}

		std::set<MessageHandler<MessageType>*> listeners;
		std::map<void*, MessageHandler<MessageType>*> handlerToBridge;
		std::vector<MessageType> queuedMessages;
	};

}
}
