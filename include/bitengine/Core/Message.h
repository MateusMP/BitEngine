#pragma once

#include "Common/TypeDefinition.h"

#define BE_MESSAGE_HANDLER(handlerFunction)		\
	std::bind(&handlerFunction, this, std::placeholders::_1)

namespace BitEngine {

	// Base Message type
	// This message is never used directly @see Message<T>
	class BaseMessage
	{
		template<typename T> friend class Message;
		BaseMessage(uint32 t) : type(t) {}

	public:
		uint32 getType() const {
			return type;
		}

	protected:
		uint32 type;

	private:
		static uint32 getNextMessageType();

	};

	// This is the template message
	// Any message type should extend this class.
	// Example:
	// class MyMessageForSomething : public class Message<MyMessageForSomething> {
	//		members...
	//		functions...
	// }
	template<typename T>
	class Message : public BaseMessage
	{
		friend class Messenger;
	public:
		Message()
			: BaseMessage(MessageType())
		{}

		static uint32 MessageType() {
			static uint32 type = getNextMessageType();
			return type;
		}
	};

}