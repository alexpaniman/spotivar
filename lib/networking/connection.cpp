#include "connection.h"
#include "axp/exceptions.h"
#include <assert.h>

uint64_t encrypte_data(uint64_t input);

sptv::connection::connection(owner parent, boost::asio::io_context& io_context, boost::asio::ip::tcp::socket socket, ts_queue<owned_message<msg_type>>& q_in, spotivar_server* owner_server)
    : asio_context(io_context), m_socket(std::move(socket)), deq_messages_in (q_in) {
        owner_type = parent;

        if (owner_type == owner::server){
            //aka generating random data to send to client on validation
            validation_out = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

            //then it will be compared
            validation_check = encrypte_data(validation_out); 

            //to handle client disconnect//
            assert (owner_server != nullptr);
            server = owner_server;
        }
        else if (owner_type == owner::client) {
            //waiting for server getting a validation handshake
            validation_in  = 0;
            validation_out = 0;
        } 
};

void sptv::connection::connect_to_server(const boost::asio::ip::tcp::resolver::results_type& endpoint) {
    boost::asio::connect(m_socket, endpoint);
}


//----------------------------------------------VALIDATION FUNCTIONS-------------------------------------------------//

uint64_t encrypte_data(uint64_t input){
//came up with realy good function for validation
    uint64_t result = input + 1;

    return result;
};

void sptv::connection::validate_client(uint32_t uid = 0) {
    if (m_socket.is_open()) {
        id = uid;
                
        //sending puzzle to client to solve//
        write_validation();

        //just after writing validation, server expects client's solved puzzle//
        read_client_validation ();
    }
};

void sptv::connection::read_client_validation() {
    if (owner_type == owner::server) {
        boost::asio::async_read(m_socket, boost::asio::buffer(&validation_in, sizeof(uint64_t)),
        [this](std::error_code ec, std::size_t length) {
            if (!ec) {
                if (validation_in == validation_check) {
                    //handling client's connection//
                    server->on_client_validated(this->shared_from_this());

                    //start to recieve requests//
                    read_header();
                }
                else {
                    //handling client's failure on server//
                    server->on_client_failed_validation(this->shared_from_this());
                }
            }
            else {
                //handling client disconnect on server//
                server->on_client_disconnect(this->shared_from_this());
            }
        });
    }
};

void sptv::connection::read_server_validation() {
    if (owner_type == owner::client) {
        try {
            boost::asio::read(m_socket, boost::asio::buffer(&validation_in, sizeof(uint64_t)));
            std::cout << "Validation message recieved\n";
        } catch (...) {
            //closing connetion//
            if (is_connected())
                m_socket.close();

            throw axp::nested_exception {"Validation failed"};
        }

        //send solved puzzle back on server
        validation_out = encrypte_data(validation_in);
        write_validation();
    }
};

void sptv::connection::write_validation() {
    boost::asio::async_write(m_socket, boost::asio::buffer(&validation_out, sizeof(uint64_t)),
    [this](std::error_code ec, std::size_t length){
        if (!ec) {
            if (owner_type == owner::client) {
                read_header ();         //when client successfuly send validation message,
                                        // it waits for incomming messages
            }
        }
        else {
            m_socket.close();
        }
    });
};

//-------------------------------------------------------------------------------------------------------------------------//



//---------------------------------------------------ASYNC IO FUNCTIONS---------------------------------------------------//

void sptv::connection::send (const message<msg_type>& msg){
    boost::asio::post (asio_context, 
    [this, msg]() {
        bool writing_message = !deq_messages_out.empty();
        deq_messages_out.push_front(msg);

        //no messages to write
        try {
            if (!writing_message) {
                write_header();
            }
        } catch (...) {
            std::cout << "SEND goes wiered!!!\n";
        }
        });

};

void sptv::connection::read_header() {
    boost::asio::async_read(m_socket, boost::asio::buffer(&temp_message.header, sizeof(message_header<msg_type>)),
    [this](std::error_code ec, std::size_t length) {
        if (!ec) {
                if (temp_message.header.size > 0) {
                    temp_message.body.resize(temp_message.header.size - sizeof(message_header<msg_type>));

                    read_body();
                }
                else {
                    add_to_incomming_queue();
                }
        }
        else {
            if (owner_type == owner::client) {
                //closing connection//
                m_socket.close();

                //throwing exception//
                throw axp::exception {"Failed to Read Header"};
            }
            else if (owner_type == owner::server) {
                //handling client disconnect//
                server->on_client_disconnect(this->shared_from_this());
            }
        }
    });
};


void sptv::connection::read_body() {
    boost::asio::async_read  (m_socket, boost::asio::buffer(temp_message.body.data(), temp_message.body.size()),
    [this](std::error_code ec, std::size_t length) {
        if (!ec) {
            add_to_incomming_queue();
        }
        else  {
            if (owner_type == owner::client) {
                //closing connection//
                m_socket.close();

                //throwing exception//
                throw axp::exception {"Failed to Read Body"};
            }
            else if (owner_type == owner::server) {
                //handling client disconnect//
                server->on_client_disconnect(this->shared_from_this());
            } 
        }
    });
};

void sptv::connection::write_header() {
    boost::asio::async_write(m_socket, boost::asio::buffer(&deq_messages_out.front().header, sizeof(message_header<msg_type>)),
    [this](std::error_code ec, std::size_t length) {
        if (!ec) {
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
        else {
            if (owner_type == owner::client) {
                //closing connection//
                disconnect();

                //throwing exception//
                throw axp::exception {"Failed to Write Header"};
            }
            else if (owner_type == owner::server) {
                //handling client disconnect//
                //throw axp::exception{"Disconnect"};
                server->on_client_disconnect(this->shared_from_this());
            } 
        }

    });
};

void sptv::connection::write_body() {
    boost::asio::async_write(m_socket, boost::asio::buffer(deq_messages_out.front().body.data(), deq_messages_out.front().body.size()),
    [this](std::error_code ec, std::size_t length) {
        if (!ec) {
            deq_messages_out.pop_front();

            if (!deq_messages_out.empty()) {
                write_header();
            }
        }
        else  {
            if (owner_type == owner::client) {
                //closing connection//
                m_socket.close();

                //throwing exception//
                throw axp::exception {"Failed to Write Body"};
            }
            else if (owner_type == owner::server) {
                //handling client disconnect//
                //throw axp::exception {"Disconnect"};
                server->on_client_disconnect(this->shared_from_this());
            } 
        }
    });
};

//--------------------------------------------------------------------------------------------------------------------//

void sptv::spotivar_server::on_client_validated (std::shared_ptr<sptv::connection> client) {
    context.post([this, client](){
        uint32_t port = client->get_port();

        //approving connection//
        std::cout << "[" << connections_map[port]->get_id() << "] Connection Approved\n";
    });
};

void sptv::spotivar_server::on_client_failed_validation(std::shared_ptr<sptv::connection> client) {
    context.post([this, client](){
        uint32_t port = client->get_port();

        //shout down connection//
        client->disconnect();

        //erase connection from server's storage map//
        connections_map.erase(port);

        //server display bad effort//
        std::cout << "[" << client->get_id() << "] Failed Validation\n";
    });
};

void sptv::spotivar_server::on_client_disconnect(std::shared_ptr<sptv::connection> client){ 
    context.post([this, client](){
        uint32_t port = client->get_port();

        //shout down connection//
        client->disconnect();

        //erase connection from server's storage map//
        connections_map.erase(port);

        std::cout << "[" << client->get_id() << "] Disconnected\n";
    });
};

//--------------------------------------------------------------------------------------------------------//