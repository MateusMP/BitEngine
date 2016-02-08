#include "Core/Messenger.h"

#include <atomic>

namespace BitEngine {

	uint32 BaseMessage::getNextMessageType()
	{
		static std::atomic<uint32> idGenerator(0);
		return ++idGenerator;
	}

	Messenger::Messenger()
	{
		m_messageQueue.reserve(1024);
	}
	
	void Messenger::Update()
	{
		// Send delayed messages
		for (BaseMessage* m : m_QueuedMessages)
		{
			SendMessage(*m);
		}

		m_QueuedMessages.clear();
		m_messageQueue.clear();
	}

	void Messenger::RegisterListener(uint32 msgType, void* handler, Callback call)
	{
		m_callbacks.emplace(msgType, Call{ handler, call });
	}

	void Messenger::UnregisterListener(void* handler)
	{
		for (auto it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
		{
			if (it->second.obj == handler)
				it = m_callbacks.erase(it);
		}
	}

	void Messenger::SendMessage(const BaseMessage& msg)
	{
		auto its = m_callbacks.equal_range(msg.getType());
		for (auto it = its.first; it != its.second; ++it)
		{
			it->second.callback(msg);
		}
	}

	void Messenger::copyMessage(const BaseMessage* msg, uint32 size)
	{
		uint32 last = m_messageQueue.size();
		m_messageQueue.resize(m_messageQueue.size() + size);

		void* start = &m_messageQueue[last];

		m_QueuedMessages.emplace(static_cast<BaseMessage*>(start));
		memcpy(start, msg, size);
	}
}