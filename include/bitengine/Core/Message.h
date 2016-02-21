#pragma once

#include "Common/TypeDefinition.h"

#define BE_MESSAGE_HANDLER(handlerFunction)		\
	std::bind(&handlerFunction, this, std::placeholders::_1)

namespace BitEngine {

	class BaseMessage
	{
		template<typename T> friend class Message;

	public:
		BaseMessage(uint32 t) : type(t) {}

		uint32 getType() const {
			return type;
		}

	protected:
		uint32 type;

	private:
		static uint32 getNextMessageType();

	};

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
