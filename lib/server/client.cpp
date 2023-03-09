//client can be easly written with pthreads
//sfml for gui

#include "client.h"
 

enum class msg_type : uint32_t
{
    Connect,
    Message,
    PingServer,
    Niga,
    SendTrack
};

class custom_client: public net::client_interface<msg_type> {
    public:
        void send_ping_message() {
            net::message<msg_type> msg;
            msg.header.id = msg_type::PingServer;

            msg << "Hello!";

            std::cout << msg;
            std::cout << msg.body.data();

            to_server_conn->send(msg);
        };

        void niga()
        {
            net::message<msg_type> msg;
            msg.header.id = msg_type::Niga;

            msg << "NIGA";

            to_server_conn->send(msg);
        };

        void send_track()
        {
            net::message<msg_type> msg;
            msg.header.id = msg_type::SendTrack;

            to_server_conn->send(msg); //what a namming, let it be Server instead 
        };
};

int main()
{
    custom_client client;
    client.connect("127.0.0.1", 7123);


    if (client.is_connected())
    {
        client.send_track();
        printf ( "\n\nMessage sent!\n");

    while (1) {
        while (client.incoming().empty()){};

        auto msg = client.incoming().pop_front().msg;

        switch (msg.header.id)
        {
            case msg_type::Niga:
                std::cout << "Server Reply: " << msg.body.data();
            break;

            case msg_type::SendTrack:
            {
                std::cout << "Track was recieved!\n";
                std::cout << "Track Size is " << msg.body.size() << "\n"; 
                
                int fd = open("New.mp3", O_WRONLY | O_CREAT);

                uint8_t* temp_buffer = (uint8_t*)calloc(msg.body.size(), sizeof(uint8_t));
                memcpy (temp_buffer, msg.body.data(), msg.body.size());

                write (fd, temp_buffer, msg.body.size()); //store the recieved track

                free (temp_buffer);
                close(fd);

                int ret = chmod("New.mp3", S_IRWXU | S_IRWXO | S_IRWXU);
            }
            break;
        }
    }
    }
    else {
        std::cout << "Server is DOWN\n"; // in case if server will fall down
    }

    sleep (5);
    printf ("Disconnecting....\n");
    

    client.disconnect();
    
    return 0;
}