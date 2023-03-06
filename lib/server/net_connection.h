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
                    {
                        id = uid;
                        ReadHeader();
                    }
                }
            };

            bool ConnectToServer(const boost::asio::ip::tcp::resolver::results_type &endpoints)
            {
                if (OwnerType == owner::client)
                {
                    boost::asio::async_connect(m_socket, endpoints,
                    [this](std::error_code ec, boost::asio::ip::tcp::endpoint endpoint)
                    {
                        if (!ec)
                        {
                            ReadHeader();
                        }
                        else
                        {
                            std::cout << "Problem occured while connecting " << ec.message() << "\n";
                        }
                    }
                    );

                }
            };

            bool Disconnect()
            {
                if (IsConnected())
                    boost::asio::post (asio_context, [this](){ m_socket.close(); });
            };

            bool IsConnected() const
            {
                return m_socket.is_open();
            };

        public:
            bool Send (const message<T>& msg) 
            {
                boost::asio::post (asio_context, 
                [this, msg]()
                {
                    bool WritingMessage = !qMessagesOut.empty();
                    qMessagesOut.push_front(msg);

                    if (!WritingMessage) //no messages to write
                    {
                        WriteHeader();
                    }
                }); 
            };


        protected:
            //asynchronasly read message header
            void ReadHeader()
            {
                boost::asio::async_read (m_socket, boost::asio::buffer(&TempMessage.header, sizeof(message_header<T>)),
                [this](std::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                            if (TempMessage.header.size > 0)
                            {
                                TempMessage.body.resize(TempMessage.header.size - sizeof(message_header<T>));
                                ReadBody();
                            }
                            else
                            {
                                AddToIncommingQueue();
                            }

                    }
                    else 
                    {
                        std::cout << "[" << id << "] " << "Failed to read header\n";
                        m_socket.close();
                    }
                });

            };

            void ReadBody()
            {
                boost::asio::async_read  (m_socket, boost::asio::buffer(TempMessage.body.data(), TempMessage.body.size()),
                [this](std::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        AddToIncommingQueue();
                    }
                    else 
                    {
                        //as above
                        std::cout << "[" << id  << "] " << "Failed to read body\n";
                        m_socket.close();
                    }
                }
                ); 

            };

            void WriteHeader()
            {
                boost::asio::async_write(m_socket, boost::asio::buffer(&qMessagesOut.front().header, sizeof(message_header<T>)),
                [this](std::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        if (qMessagesOut.front().body.size() > 0)
                        {
                            WriteBody();
                        }
                        else 
                        {
                            qMessagesOut.pop_front();

                            if (!qMessagesOut.empty())
                            {
                                WriteHeader();
                            }
                        }
                    }
                    else 
                    {
                        std::cout << "[ " << id << " ]" << "Write Header Failed\n";
                        m_socket.close();
                    }

                });

            };

            void WriteBody()
            {
                boost::asio::async_write(m_socket, boost::asio::buffer(qMessagesOut.front().body.data(), qMessagesOut.front().body.size()),
                [this](std::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        qMessagesOut.pop_front();

                        if (!qMessagesOut.empty())
                        {
                            WriteHeader();
                        }
                    }
                    else 
                    {
                        std::cout << "[ " << id << " ]" << "Write Body Failed\n";
                        m_socket.close();
                    }
                });

                 
            };
        

            void AddToIncommingQueue()
            {
                if (OwnerType == owner::server)
                    qMessagesIn.push_back({ this->shared_from_this(), TempMessage });
                else
                    qMessagesIn.push_back({ nullptr, TempMessage });
                
                //std::cout << "Message put in private queue\n";

                ReadHeader();
            };


        protected:
            boost::asio::ip::tcp::socket m_socket;
            boost::asio::io_context& asio_context;

            //this queue consists only messages that are need to be sent 
            //to the remote side of connection
            ts_queue<message<T>> qMessagesOut;

            //this message queue consists messages that indicate 
            //who in particular sent this message, namely reference to "connection" 
            ts_queue<owned_message<T>>& qMessagesIn; //this is not connection's queue, client or server has it

            message<T> TempMessage;

            owner OwnerType = owner::server;
            uint32_t id = 0;

    };
}