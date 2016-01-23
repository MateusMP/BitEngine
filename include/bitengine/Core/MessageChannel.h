#pragma once

#include <vector>
#include <functional>

namespace BitEngine{

/**
 * Synchronized channel
 * Messages sent through this channel are processed immediately by the same thread
 */
class Channel
{
    private:
        template<typename MessageType>
        class MessageChannel;

    public:
        template<typename MessageType, typename tListener>
        static void AddListener(tListener *listener)
        {
            MessageChannel<MessageType>::Inst().Add(listener);
        }

        template<typename MessageType, typename tListener>
        static void RemoveListener(tListener *listener)
        {
            MessageChannel<MessageType>::Inst().Remove(listener);
        }

        template<typename MessageType>
        static void Broadcast(const MessageType& msg)
        {
            MessageChannel<MessageType>::Inst().Broadcast(msg);
        }

    private:
        template<typename MessageType>
        class MessageChannel
        {
            public:
                typedef std::function<void(const MessageType&)> FuncMsgT;

                static MessageChannel& Inst()
                {
                    static MessageChannel instance;
                    return instance;
                }

                template <typename tListener>
                void Add(tListener *listener)
                {
                    m_Handlers.push_back([listener] (const MessageType& msg) { (*listener).Message(msg); });
                }

                template <typename tListener>
                void Remove(tListener *listener)
                {
                    ListenerComparator<tListener> listcmp(listener);
                    m_Handlers.remove_if(listcmp);
                }

                void Broadcast(const MessageType& msg)
                {
                    // Local copy of the handlers list to avoid an invalid
                    // list if the hadler unregister itself during broadcast
                    std::vector<FuncMsgT> listCopy = m_Handlers;

                    for ( auto& it : listCopy ){
                        it(msg);
                    }
                }

            private:
                std::vector<FuncMsgT> m_Handlers;

                template <typename tListener>
                class ListenerComparator{

                    public:
                        ListenerComparator(tListener *listener);

                        bool operator()(tListener *ptr) {
                            return ptr == m_listener;
                        }

                    private:
                        tListener m_listener;
                };
        };
};


}

