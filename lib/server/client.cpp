//client can be easly written with pthreads
//sfml for gui

#include "client.h"
 

enum class MsgType : uint32_t
{
    Connect,
    Message,
    PingServer,
    Niga
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

        connection->Send(msg);
    };
    void Niga()
    {
        net::message<MsgType> msg;
        msg.header.id = MsgType::Niga;

        msg << "NIGA";

        connection->Send(msg);
    }

};

int main()
{

    CustomClient client;
    client.Connect ("127.0.0.1", 7123);


    while (true)
    {
        if (client.IsConnected())
        {
            //client.SendPingMessage(); //greating with server
            client.Niga();
            printf ( "\n\nMessage sent!\n");

            while (client.Incoming().empty()){};

            auto msg = client.Incoming().pop_front().msg;

            switch (msg.header.id)
            {
                case MsgType::Niga:
                    std::cout << "Server Reply: " << msg.body.data();
                break;
            }
        }
        else 
        {
            std::cout << "Server is DOWN\n"; // in case if server will fall down
        }
        sleep (5);
    }


    //printf ("Disconnecting....\n");
    

    client.Disconnect();
    
    return 0;
}