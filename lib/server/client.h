#pragma once

#include "common_include.h"
#include "thread_safe_queues.h"
#include "net_message.h"
#include "net_connection.h"


namespace net
{
    template <typename T>
    class client_interface
    {
    public:
        client_interface() : socket(context)
        {
            //init the socket with context, let asio to perform something
        };

        virtual ~client_interface()
        {
            Disconnect();
        };

         private:
            ts_queue<owned_message<T>> qMessagesIn;

        protected:
            boost::asio::io_context context;
            std::thread ThrContext;  //thread is needed for asio to perform its tasks
           
            boost::asio::ip::tcp::socket   socket;
            std::unique_ptr<connection<T>> connection;


        public:
            bool Connect(const std::string& host, const uint16_t port)
            {
                try
                {
                    boost::asio::ip::tcp::resolver resolver(context);
                    boost::asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(host, std::to_string(port));

                    connection = std::make_unique<net::connection<T>>(
                        net::connection<T>::owner::client,
                        context,
                        boost::asio::ip::tcp::socket(context), qMessagesIn);

                    connection->ConnectToServer(endpoint);

                    ThrContext = std::thread([this]() { context.run(); });
                }
                catch(const std::exception& e)
                {
                    std::cerr << "Client Exception" << e.what() << '\n';
                    return false;
                }
                
                return false;
            }

            void Disconnect()
            {
                if (IsConnected())
                {
                    connection->Disconnect();
                }
                
                context.stop();

                if (ThrContext.joinable())
                    ThrContext.join();

                connection.release();
            };

            bool IsConnected()
            {
                if (connection)
                    return connection->IsConnected();
                else 
                    return false;
            };

            ts_queue<owned_message<T>>& Incoming ()
            {
                return qMessagesIn;
            };
    };
}