#pragma once

#include "net_message.h"
#include "thread_safe_queues.h"

namespace net
{
    template <typename T>
    class connection : public std::enable_shared_from_this<connection<T>>
    {
        public:
        enum class owner
        {
            server,
            client
        };

        connection(owner parent, boost::asio::io_context& IO_context, boost::asio::ip::tcp::socket socket, ts_queue<owned_message<T>>& qIn)
                : asio_context(IO_context), m_socket(std::move(socket)), qMessagesIn (qIn) 
        {
                OwnerType = parent;
        }; //constructor

        virtual ~connection(){}; //distructor

        uint32_t GetID() const
        {
            return id;
        };

        public:
            void ConnectToClient(uint32_t uid = 0)
            {
                if (OwnerType == owner::server)
                {
                    if (m_socket.is_open())
                        id = uid;
                }
            }

            bool ConnectionToServer(){};
            bool Disconnect(){};

            bool IsConnect() const
            {
                return m_socket.is_open();
            };

        public:
            bool Send (const message<T>& msg) {};

        protected:
            boost::asio::ip::tcp::socket m_socket;
            boost::asio::io_context& asio_context;

            //this queue consists only messages that are need to be sent 
            //to the remote side of connection
            ts_queue<message<T>> qMessagesOut;

            //this message queue consists messages that indicate 
            //who in particular sent this message, namely reference to "connection" 
            ts_queue<owned_message<T>>& qMessagesIn; //this is not connection's queue, client or server has it

            owner OwnerType = owner::server;
            uint32_t id = 0;

    };
}