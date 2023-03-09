#include "server.h"
#include "Text.h"
#include <sys/stat.h>
#include <errno.h>

typedef struct stat my_stat;

enum class msg_type : uint32_t
{
    Connect,
    Message,
    PingServer,
    Niga,
    SendTrack
};


class custom_server: public net::server_interface<msg_type>
{
    public:
        custom_server(uint16_t port) : net::server_interface<msg_type>(port) {
        };

    protected:
        virtual bool on_client_connect(std::shared_ptr<net::connection<msg_type>> client) {
            return true;
        };

        virtual bool on_message(std::shared_ptr<net::connection<msg_type>> client, net::message<msg_type>& msg) {
            switch (msg.header.id) {
                case msg_type::PingServer:
                {
                    std::cout << "[" << client->get_id() << "] Server Recieved Message!\n";
                    std::cout << "[" << client->get_id() << "] " << msg.body.data() << "\n";

                    net::message<msg_type> response;
                    response.header.id = msg_type::PingServer;
                    
                    response << "Hello from the server!\n"; 
                    client->send(response);
                }
                break;

                case msg_type::Niga:
                {
                    std::cout << "[" << client->get_id() << "] Server Recieved Message!\n";
                    std::cout << "[" << client->get_id() << "] " << msg.body.data() << "\n";

                    net::message<msg_type> response;
                    response.header.id = msg_type::Niga;
                    
                    response << "Shut UP woman!"; 
                    client->send(response);
                }
                break;

                case msg_type::SendTrack:
                {
                    FILE* mp3 = fopen("Ya_Pizdatiy.mp3", "rb"); //WHAT?? WHY DOES IT RETURN NULL??

                    if (!mp3) {
                        perror("fopen ");
                        /// die
                    }
                    printf ("the file address %p\n", mp3);

                    long long int file_size = GetFileSize(mp3);
                    printf ("file size cheeeck: %llu\n", file_size);

                    uint8_t* temp_buffer = (uint8_t*)calloc(file_size, sizeof(uint8_t));
                    fread (temp_buffer, sizeof(uint8_t), file_size, mp3);

                    net::message<msg_type> track;
                    track.header.id = msg_type::SendTrack;

                    track.body.resize(file_size);
                    memcpy (track.body.data(), temp_buffer, file_size);

                    track.header.size = track.size(); //update message size to send

                    client->send(track);

                    free  (temp_buffer);
                    fclose(mp3);
                }
                break;
            }

            return true;
        }
};


int main ()
{
    custom_server server(7123);
    server.start();

    while (true) {
        server.update();
    }

    return 0;
}