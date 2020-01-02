#pragma once

#include <set>
#include <vector>
#include <functional>
#include <map>
#include <memory>
#include <typeindex>
#include <cstring>

#include "BitEngine/Core/Assert.h"
#include "BitEngine/Common/TypeDefinition.h"

namespace BitEngine {

template<typename EventType>
class BE_API Messenger : public NonCopyable, NonAssignable
{
private:

    using call_type = std::function<void(const EventType&)>;

    template <class Handler>
    using member_func_t = void (Handler::*)(const EventType&);

    // Subscription Handle
    typedef u32 SubsHandle;

public:

    Messenger() : handles(0) {
        m_subscribers.reserve(32);
    }

    template<typename Handler>
    SubsHandle subscribe(member_func_t<Handler> func, Handler* handler) {
        return _subscribe<Handler>(func, handler);
    }

    /**
    * Subscriber a handler to an EventType.
    * The handler method should follow the signature: void (const EventType&)
    */
    SubsHandle subscribe(call_type callable) {
        SubsHandle handle = ++handles;
        m_subscribers.emplace_back(CallbackWrapper(callable), handle);
        return handle;
    }

    void unsubscribe(SubsHandle handle) {
        for (auto it = m_subscribers.begin(); it != m_subscribers.end(); ++it) {
            if (it->handle == handle) {
                m_subscribers.erase(it);
                break;
            }
        }
    }

    /**
    * Emit a message to all subscribers. Handler are called immediately.
    */
    void emit(const EventType& event) {
        for (auto& receiver : m_subscribers) {
            receiver.callback(event);
        }
    }

    /**
    * Emit a message that is deferred until a dispatch() call.
    */
    void enqueue(const EventType& event)
    {
        m_enqueuedEventData.emplace_back(event)
    }

    /**
    * Dispatch all enqueued messages in the order that they were enqueued.
    */
    void dispatch() {
        m_enqueuedEventDataBuffer.swap(m_enqueuedEventData);

        for (const EventType& data : m_enqueuedEventDataBuffer) {
            for (auto& receiver : m_subscribers) {
                receiver.callback(data);
            }
        }
        m_enqueuedEventDataBuffer.clear();
    }

private:

    template<typename Handler>
    SubsHandle _subscribe(member_func_t<Handler> func, Handler* handler) {
        auto f = [handler, func](const EventType& ev) -> void { (handler->*func)(ev); };
        return subscribe(f);
    }

    struct callback_handle {
        callback_handle(call_type c, SubsHandle h) : callback(c), handle(h) {
        }
        call_type callback;
        SubsHandle handle;
    };

    std::vector< callback_handle > m_subscribers;
    std::vector<EventType> m_enqueuedEventData;
    std::vector<EventType> m_enqueuedEventDataBuffer;
    u32 handles;

    struct CallbackWrapper
    {
        CallbackWrapper(const call_type& callable) : m_callable(callable) {}

        void operator() (const EventType& msg) {
            m_callable(msg);
        }

        call_type m_callable;
    };

public:

    class ScopedSubscription {
    public:
        template<typename Handler>
        ScopedSubscription(Messenger<EventType>& msg, member_func_t<Handler> func, Handler* handler)
            : messenger(msg) {
            handle = messenger.subscribe(func, handler);
        }
        ScopedSubscription(Messenger<EventType>& msg, call_type call)
            : messenger(msg) {
            handle = messenger.subscribe(call);
        }
        ScopedSubscription(Messenger<EventType>& msg, u32 h)
            : messenger(msg), handle(h) {
        }
        ~ScopedSubscription() {
            messenger.unsubscribe(handle);
        }
        u32 handle;
        Messenger<EventType>& messenger;
    };
};

}
