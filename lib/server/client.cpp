//client can be easly written with pthreads
//sfml for gui

#include "client.h"
 

enum class MsgType : uint32_t
{
    Connect,
    Message,
    PingServer,
    Niga,
    SendTrack
};

class CustomClient : public net::client_interface<MsgType>
{

public:
    void SendPingMessage()
    {
        net::message<MsgType> msg;
        msg.header.id = MsgType::PingServer;

        msg << "Hello!";

        std::cout << msg;
        std::cout << msg.body.data();

        ToServerConn->Send(msg);
    };
    void Niga()
    {
        net::message<MsgType> msg;
        msg.header.id = MsgType::Niga;

        msg << "NIGA";

        ToServerConn->Send(msg);
    };

    void SendTrack()
    {
        net::message<MsgType> msg;
        msg.header.id = MsgType::SendTrack;

        ToServerConn->Send(msg); //what a namming, let it be Server instead 
    }


};

int main()
{

    CustomClient client;
    client.Connect ("127.0.0.1", 7123);


    if (client.IsConnected())
    {
        //client.SendPingMessage(); //greating with server
        client.SendTrack();
        printf ( "\n\nMessage sent!\n");

    while (1)
    {
        while (client.Incoming().empty()){};

        auto msg = client.Incoming().pop_front().msg;

        switch (msg.header.id)
        {
            case MsgType::Niga:
                std::cout << "Server Reply: " << msg.body.data();
            break;

            case MsgType::SendTrack:
            {
                std::cout << "Track was recieved!\n";
                std::cout << "Track Size is " << msg.body.size() << "\n"; 
                
                int fd = open("New.mp3", O_WRONLY | O_CREAT);
                //full path is needed??

                uint8_t* TempBuffer = (uint8_t*)calloc(msg.body.size(), sizeof(uint8_t));
                memcpy (TempBuffer, msg.body.data(), msg.body.size());

                write (fd, TempBuffer, msg.body.size()); //store the recieved track

                free (TempBuffer);
                close(fd);
            }
            break;
        }
    }
    }
    else 
    {
        std::cout << "Server is DOWN\n"; // in case if server will fall down
    }

    sleep (5);
    printf ("Disconnecting....\n");
    

    client.Disconnect();
    
    return 0;
}