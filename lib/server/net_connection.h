#pragma once

#include "net_message.h"
#include "thread_safe_queues.h"

namespace sptv
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

            if (owner_type == owner::server){
                //aka generating random data to send to client on validation
                validation_out = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

                //then it will be compared
                validation_check = encrypte_data(validation_out); 
            }
            else {
                //waiting for server getting a validation handshake
                validation_in  = 0;
                validation_out = 0;
            } 
        }; //constructor

        virtual ~connection(){}; //destructor

        uint32_t get_id() const{
            return id;
        };
    
        void connect_to_client(uint32_t uid = 0) {
            if (owner_type == owner::server) {
                if (m_socket.is_open()) {
                    id = uid;
                    
                    write_validation();
                    read_validation ();
                }
            }
        };

        void connect_to_server(const boost::asio::ip::tcp::resolver::results_type &endpoints) {
            if (owner_type == owner::client) {
                boost::asio::async_connect(m_socket, endpoints,
                    [this](std::error_code ec, boost::asio::ip::tcp::endpoint endpoint){
                    //now we expect validation message first
                    read_validation();
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

        void write_validation(){
            boost::asio::async_write(m_socket, boost::asio::buffer(&validation_out, sizeof(uint64_t)),
            [this](std::error_code ec, std::size_t length){
                if (!ec) {
                    if (owner_type == owner::client)
                        read_header (); //when client successfuly send validation message,
                                        // it waits for incomming messages
                }
                else {
                    m_socket.close(); //just shout down the connection
                }
                
            });
        };

        void read_validation(/*class sptv::spotivar_server* server = nullptr*/){
            boost::asio::async_read(m_socket, boost::asio::buffer(&validation_in, sizeof(uint64_t)),
            [this](std::error_code ec, std::size_t length){
                if (!ec) {
                    if (owner_type == owner::server) {
                        if (validation_in == validation_check)
                        {
                            std::cout << "Client Validated\n";
                            //server->on_client_validated(this->shared_from_this());

                            read_header();
                        }
                        else
                        {
                            std::cout << "Client FAILED Validation\n";
                            m_socket.close();
                        }
                        
                    }
                    else {
                        validation_out = encrypte_data(validation_in);
                        write_validation();
                    }
                }
                else {
                    std::cout << "Read Validation failed, closing connection ...\n";
                    m_socket.close();
                }
            });
        }
    

        void add_to_incomming_queue() {
            if (owner_type == owner::server)
                deq_messages_in.push_back({ this->shared_from_this(), temp_message });
            else
                deq_messages_in.push_back({ nullptr, temp_message });
            
            //std::cout << "Message put in private queue\n";

            read_header();
        };

        uint64_t encrypte_data(uint64_t input){
            //come up with a realy great function for validation
            uint64_t result = input + 1;

            return result;
        }


        boost::asio::ip::tcp::socket m_socket;
        boost::asio::io_context&     asio_context;

        //this queue consists only messages that are need to be sent 
        //to the remote side of connection
        ts_queue<message<T>> deq_messages_out;

        //this message queue consists messages that indicate 
        //who in particular sent this message, namely reference to "connection" 
        ts_queue<owned_message<T>>& deq_messages_in; //this is not connection's queue, client or server has it

        message<T> temp_message;

        owner owner_type = owner::server;
        uint32_t id = 0;

        //validation
        uint64_t validation_out   = 0;
        uint64_t validation_in    = 0;
        uint64_t validation_check = 0;

    };
}