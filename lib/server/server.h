#pragma once

#include "common_include.h"
#include "thread_safe_queues.h"
#include "net_connection.h"
#include "net_message.h"


namespace net
{
    template<typename T>
    class server_interface
    {
    public:
        server_interface(uint16_t port)
             : AsioAcceptor(Context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
        {

        };

        virtual ~server_interface()
        {
            Finish();
        };

        bool Start()  
        {
            try
            {
                WaitForClientConnection();

                ThreadContext = std::thread([this]() { Context.run(); });
            }
            catch(const std::exception& e)
            {
                std::cerr << "[SERVER] Exception " << e.what() << '\n';
                return false;
            }

            std::cout << "[SERVER] started\n";
            return true;
            
        };
         
        bool Finish()
        {
            Context.stop();

            if (ThreadContext.joinable())
                ThreadContext.join();

            std::cout << "[SERVER] Stopped!\n";

            return true;
        };

        void WaitForClientConnection ()
        {
            AsioAcceptor.async_accept(
                [this](std::error_code error, boost::asio::ip::tcp::socket socket)
                {
                    if (!error)
                    {
                        std::cout << "New connect " << socket.remote_endpoint() << "\n"; //this method return ip of connection

                        std::shared_ptr<connection<T>> new_connection =  //create new object - new connection
                             std::make_shared<connection<T>>(connection<T>::owner::server, Context, std::move(socket), qMessageIn);

                        if(OnClientConnect(new_connection))
                        {
                            deqConnections.push_back(std::move(new_connection));
                            deqConnections.back()->ConnectToClient(ID_counter ++); // here i start readheaders

                            std::cout << "[" << deqConnections.back()->GetID() << "] Connection Approved\n";

                        } //establish connection
                        else
                            std::cout << "Connection Denied\n";
 
                    }
                    else
                        std::cout << "[SERVER] Error while accept\n";

                    //anyway, start to try again connect new user
                    WaitForClientConnection();
                }
            );
        };

        void MessageClient (std::shared_ptr<connection<T>> client, const message<T>& msg)
        {
            if (client && client->IsConnected())
            {
                client->Send(msg);
            }
            else
            {
                OnClientDisconnect(client);
                client.reset();

                deqConnections.erase(std::remove(deqConnections.begin(), deqConnections.end(), client), deqConnections.end());

            }

        };

        void MessageAllClients (const message<T>& msg, std::shared_ptr<connection<T>> pIgnor = nullptr)
        {
            bool InvalidClientExists = false;

            for (auto& client : deqConnections)
            {
                if (client && client->IsConnected())
                {
                    if (client != pIgnor)
                        client->Send(msg);
                }
                else
                {
                    OnClientDisconnect(client);
                    client.reset();
                    InvalidClientExists = true;
                }
            }

            if (InvalidClientExists)
                deqConnections.erase(std::remove(deqConnections.begin(), deqConnections.end(), nullptr), deqConnections.end());

        };

        void Update(size_t nMaxMessage = -1)
        {
            size_t nMessageCount = 0;
            while (nMessageCount < nMaxMessage && !qMessageIn.empty())
            {
                auto msg = qMessageIn.pop_front();

                OnMessage(msg.remote, msg.msg);

                nMessageCount ++;
            }
        }

    protected:

        //checker that will deside weather to connect user or not 
        virtual bool OnClientConnect(std::shared_ptr<connection<T>> client)
        {
            return false;
        };

        //called when client disconnect, this thing will detect did anyone disconnect
        virtual bool OnClientDisconnect(std::shared_ptr<connection<T>> client)
        {
            return false;
        };

        virtual bool OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg)
        {
            return false;
        };
    
    protected:
        ts_queue<owned_message<T>> qMessageIn;
        std::deque<std::shared_ptr<connection<T>>> deqConnections;

        boost::asio::io_context Context;
        std::thread ThreadContext;
  
        boost::asio::ip::tcp::acceptor AsioAcceptor; //this class will get users sockets

        uint32_t ID_counter = 10000;

    };

}