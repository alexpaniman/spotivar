#pragma once

#include "net_message.h"
#include "thread_safe_queues.h"

namespace net
{
    template <typename T>
    class connection: public std::enable_shared_from_this<connection<T>> {

    public:
        enum class owner{
            server,
            client
        };

        connection(owner parent, boost::asio::io_context& io_context, boost::asio::ip::tcp::socket socket, ts_queue<owned_message<T>>& q_in)
                : asio_context(io_context), m_socket(std::move(socket)), deq_messages_in (q_in){
                owner_type = parent;
        }; //constructor

        virtual ~connection(){}; //destructor

        uint32_t get_id() const{
            return id;
        };
    
        void connect_to_client(uint32_t uid = 0) {

            if (owner_type == owner::server) {
                if (m_socket.is_open()) {
                    id = uid;
                    read_header();
                }
            }
        };

        void connect_to_server(const boost::asio::ip::tcp::resolver::results_type &endpoints) {
            if (owner_type == owner::client) {
                boost::asio::async_connect(m_socket, endpoints,
                    [this](std::error_code ec, boost::asio::ip::tcp::endpoint endpoint){
                    //if exeption, catch at level higher
                    read_header();
            });
            }
        };

        bool disconnect(){
            if (is_connected())
                boost::asio::post (asio_context, [this](){ m_socket.close(); });

            return true;
        };

        bool is_connected() const {
            return m_socket.is_open();
        };

        void send (const message<T>& msg){
            boost::asio::post (asio_context, 
            [this, msg]() {
                bool writing_message = !deq_messages_out.empty();
                deq_messages_out.push_front(msg);

                if (!writing_message){                        //no messages to write
                    write_header();
                }
            }); 
        };


    protected:
        //FOR NOW: I WILL LEAVE EXCEPTIONS FOR DEBUG
        //THEN THEY WILL BE REMOVED

        //asynchronasly read message header
        void read_header(){
            boost::asio::async_read(m_socket, boost::asio::buffer(&temp_message.header, sizeof(message_header<T>)),
            [this](std::error_code ec, std::size_t length) {
                if (!ec)
                {
                        if (temp_message.header.size > 0) {
                            temp_message.body.resize(temp_message.header.size - sizeof(message_header<T>));
                            read_body();
                        }
                        else {
                            add_to_incomming_queue();
                        }

                }
                else 
                {
                    std::cout << "[" << id << "] " << "Failed to read header\n";
                    m_socket.close();
                }
            });

        };

        void read_body() {
            boost::asio::async_read  (m_socket, boost::asio::buffer(temp_message.body.data(), temp_message.body.size()),
            [this](std::error_code ec, std::size_t length) {
                if (!ec)
                {
                    add_to_incomming_queue();
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

        void write_header() {
            boost::asio::async_write(m_socket, boost::asio::buffer(&deq_messages_out.front().header, sizeof(message_header<T>)),
            [this](std::error_code ec, std::size_t length) {
                if (!ec)
                {
                    if (deq_messages_out.front().body.size() > 0) {
                        write_body();
                    }
                    else {
                        deq_messages_out.pop_front();

                        if (!deq_messages_out.empty()) {
                            write_header();
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

        void write_body() {
            boost::asio::async_write(m_socket, boost::asio::buffer(deq_messages_out.front().body.data(), deq_messages_out.front().body.size()),
            [this](std::error_code ec, std::size_t length) {
                if (!ec)
                {
                    deq_messages_out.pop_front();

                    if (!deq_messages_out.empty()) {
                        write_header();
                    }
                }
                else 
                {
                    std::cout << "[ " << id << " ]" << "Write Body Failed\n";
                    m_socket.close();
                }
            });
        };
    

        void add_to_incomming_queue() {
            if (owner_type == owner::server)
                deq_messages_in.push_back({ this->shared_from_this(), temp_message });
            else
                deq_messages_in.push_back({ nullptr, temp_message });
            
            //std::cout << "Message put in private queue\n";

            read_header();
        };


        boost::asio::ip::tcp::socket m_socket;
        boost::asio::io_context& asio_context;

        //this queue consists only messages that are need to be sent 
        //to the remote side of connection
        ts_queue<message<T>> deq_messages_out;

        //this message queue consists messages that indicate 
        //who in particular sent this message, namely reference to "connection" 
        ts_queue<owned_message<T>>& deq_messages_in; //this is not connection's queue, client or server has it

        message<T> temp_message;

        owner owner_type = owner::server;
        uint32_t id = 0;

    };
}