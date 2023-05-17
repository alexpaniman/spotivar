#pragma once

#include "net_message.h"
#include "thread_safe_queues.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>

namespace sptv {
    class connection: public std::enable_shared_from_this<connection> {

    public:
        enum class owner{
            server,
            client
        };

        connection(owner parent, boost::asio::io_context& io_context, boost::asio::ip::tcp::socket socket, ts_queue<owned_message<msg_type>>& q_in, spotivar_server* owner_server = nullptr);

        void connect_to_server(const boost::asio::ip::tcp::resolver::results_type& enpoint);

        void disconnect() {
            //closing connection//
            m_socket.close();
        }

        uint32_t get_port (){
            return m_socket.remote_endpoint().port();
        }
    
        //validation functions//
        void validate_client(uint32_t uid);
        void read_client_validation();
        void read_server_validation();
        void write_validation();
        //--------------------//

        void send (const message<msg_type>& msg);

        //--------------------------//
        bool is_connected() const {
            return m_socket.is_open();
        };
        uint32_t get_id() const{
            return id;
        };
        //--------------------------//

    protected:
        //asynch recieve//
        void read_header();
        void read_body();
        //--------------//

        //asynch send//
        void write_header();
        void write_body();
        //-----------//
    
        void add_to_incomming_queue() {
            if (owner_type == owner::server)
                deq_messages_in.push_back({ this->shared_from_this(), temp_message });
            else
                deq_messages_in.push_back({ nullptr, temp_message });
            

            read_header();
        };



        boost::asio::ip::tcp::socket m_socket;
        boost::asio::io_context&     asio_context;

        //this queue consists only messages that are need to be sent 
        //to the remote side of connection
        ts_queue<message<msg_type>> deq_messages_out;

        //this message queue consists messages that indicate 
        //who in particular sent this message, namely reference to "connection" 
        ts_queue<owned_message<msg_type>>& deq_messages_in; //this is not connection's queue, client or server has it

        message<msg_type> temp_message;

        owner owner_type  = owner::server;
        spotivar_server* server = nullptr;

        uint32_t id = 0;

        //validation
        uint64_t validation_out   = 0;
        uint64_t validation_in    = 0;
        uint64_t validation_check = 0;

    };
};