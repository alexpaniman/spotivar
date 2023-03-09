#pragma once

#include "common_include.h"
#include "thread_safe_queues.h"
#include "net_message.h"
#include "net_connection.h"


namespace net
{
    template <typename T>
    class client_interface {
 
    public:
        client_interface() : socket(context){
            //init the socket with context, let asio to perform something
        };

        virtual ~client_interface(){
            disconnect();
        };

            //transfer the exception on a higher level
        bool connect(const std::string& host, const uint16_t port){
            boost::asio::ip::tcp::resolver resolver(context);
            boost::asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(host, std::to_string(port));

            to_server_conn = std::make_unique<net::connection<T>>(
                net::connection<T>::owner::client,
                context,
                boost::asio::ip::tcp::socket(context), deq_messages_in);

            to_server_conn->connect_to_server(endpoint);
            thr_context = std::thread([this]() { context.run(); });
            
            return true;
        };

        void disconnect(){
            if (is_connected()){
                to_server_conn->disconnect();
            }
            
            context.stop();

            if (thr_context.joinable())
                thr_context.join();

            to_server_conn.release();
        };

        bool is_connected(){
            if (to_server_conn)
                return to_server_conn->is_connected();
            else 
                return false;
        };

        ts_queue<owned_message<T>>& incoming (){
            return deq_messages_in;
        };

    protected:
        boost::asio::io_context context;
        std::thread             thr_context;  //thread is needed for asio to perform its tasks
        
        boost::asio::ip::tcp::socket   socket;
        std::unique_ptr<connection<T>> to_server_conn;

    private:
        ts_queue<owned_message<T>> deq_messages_in;

    };
}