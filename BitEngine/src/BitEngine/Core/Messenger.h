#pragma once

#include <set>
#include <vector>
#include <functional>
#include <map>
#include <memory>
#include <typeindex>
#include <cstring>

#include "BitEngine/Common/Vector.h"

#include "BitEngine/Core/Assert.h"
#include "BitEngine/Common/TypeDefinition.h"

namespace BitEngine {

template<typename EventType, ptrsize MaxSubs = 4, ptrsize MaxQueued = 4>
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
    }

    template<typename Handler>
    SubsHandle subscribe(member_func_t<Handler> func, Handler* handler) {
        auto f = [handler, func](const EventType& ev) -> void { (handler->*func)(ev); };
        return subscribe(f);
    }

    /**
    * Subscriber a handler to an EventType.
    * The handler method should follow the signature: void (const EventType&)
    */
    SubsHandle subscribe(call_type callable) {
        SubsHandle handle = ++handles;
        m_subscribers.emplace_back(callback_handle{ CallbackWrapper(callable), handle });
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
    void enqueue(const EventType& event) {
        m_enqueuedEventData.emplace_back(event)
    }

    /**
    * Dispatch all enqueued messages in the order that they were enqueued.
    */
    void dispatch() {
        for (const EventType& data : m_enqueuedEventData) {
            emit(data);
        }
    }

private:
    struct callback_handle {
        call_type callback;
        SubsHandle handle;
    };

    TightFixedVector<callback_handle, MaxSubs> m_subscribers;
    TightFixedVector<EventType, MaxQueued> m_enqueuedEventData;
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
