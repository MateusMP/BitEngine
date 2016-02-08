#pragma once

#include "Common/TypeDefinition.h"


#define CREATE_MESSAGE_TYPE(className)										\
	template<> uint32 BitEngine::Message<className>::MessageType(){					\
		static uint32 ID = BitEngine::BaseMessage::getNextMessageType();	\
		return ID;															\
	}

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

	private:
		static uint32 MessageType();
	};

}
