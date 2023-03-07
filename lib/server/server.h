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
        server_interface(uint16_t port) // add start in the server constructor, so when it inited - server starts
             : asio_acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)){
        };

        virtual ~server_interface(){
            finish();
        };

        //move catch exception on a higher level
        bool start(){
            wait_for_client_connection();

            thread_context = std::thread([this]() { context.run(); });
            std::cout << "[SERVER] started\n";

            return true;
        };
         
        bool finish(){
            context.stop();

            if (thread_context.joinable())
                thread_context.join();

            std::cout << "[SERVER] Stopped!\n";

            return true;
        };

        void wait_for_client_connection(){
            asio_acceptor.async_accept(
                [this](boost::asio::ip::tcp::socket socket){
                        std::cout << "New Connect " << socket.remote_endpoint() << "\n"; //this method return ip of connection

                        std::shared_ptr<connection<T>> new_connection =  //create new object - new connection
                                std::make_shared<connection<T>>(connection<T>::owner::server, context, std::move(socket), deq_message_in);

                        if(on_client_connect(new_connection)){
                            deq_connections.push_back(std::move(new_connection));
                            deq_connections.back()->connect_to_client(id_counter ++); // here i start readheaders

                            std::cout << "[" << deq_connections.back()->get_id() << "] Connection Approved\n";

                        } //establish connection
                        else
                            std::cout << "Connection Denied\n";
                    }

                    //anyway, start to try again connect new user
                    wait_for_client_connection();
            );};
            
        void message_client(std::shared_ptr<connection<T>> client, const message<T>& msg){
            if (client && client->is_connected()){
                client->send(msg);
            }
            else {
                on_client_disconnect(client);
                client.reset();

                deq_connections.erase(std::remove(deq_connections.begin(), deq_connections.end(), client), deq_connections.end());
            }
        };

        void message_all_clients(const message<T>& msg, std::shared_ptr<connection<T>> ignor_client = nullptr){
            bool invalid_client_exists = false;

            for (auto& client : deq_connections){
                if (client && client->is_connected()){
                    if (client != ignor_client)
                        client->send(msg);
                }
                else {
                    on_client_disconnect(client);
                    client.reset();
                    invalid_client_exists = true;
                }
            }

            if (invalid_client_exists)
                deq_connections.erase(std::remove(deq_connections.begin(), deq_connections.end(), nullptr), deq_connections.end());

        };

        void update(size_t max_messages = -1){
            size_t message_count = 0;

            while (message_count < max_messages && !deq_message_in.empty()) {
                auto msg = deq_message_in.pop_front();
                on_message(msg.remote, msg.msg);

                message_count ++;
            }
        };

    protected:
        ts_queue<owned_message<T>> deq_message_in;
        std::deque<std::shared_ptr<connection<T>>> deq_connections;

        boost::asio::io_context context;
        std::thread             thread_context;
  
        boost::asio::ip::tcp::acceptor asio_acceptor; //this class will get users sockets

        uint32_t id_counter = 10000;

        //checker that will deside weather to connect user or not 
        virtual bool on_client_connect(std::shared_ptr<connection<T>> client){
            return false;
        };

        //called when client disconnect, this thing will detect did anyone disconnect
        virtual bool on_client_disconnect(std::shared_ptr<connection<T>> client){
            return false;
        };

        virtual bool on_message(std::shared_ptr<connection<T>> client, message<T>& msg){
            return false;
        };
    };
};
