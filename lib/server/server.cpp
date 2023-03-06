#include "server.h"

enum class MsgType : uint32_t
{
    Connect,
    Message,
    PingServer,
    Niga
};


class CustomServer : public net::server_interface<MsgType>
{
    public:
        CustomServer(uint16_t port) : net::server_interface<MsgType>(port)
        {
        };

    protected:
        virtual bool OnClientConnect (std::shared_ptr<net::connection<MsgType>> client)
        {
            return true;
        };

        virtual bool OnMessage (std::shared_ptr<net::connection<MsgType>> client, net::message<MsgType>& msg)
        {
            switch (msg.header.id)
            {
                case MsgType::PingServer:
                {
                    std::cout << "[" << client->GetID() << "] Server Recieved Message!\n";
                    std::cout << "[" << client->GetID() << "] " << msg.body.data() << "\n";
                    
                    printf ("The size of recieved message: [%d]\n", msg.header.size - sizeof(net::message_header<MsgType>));

                    net::message<MsgType> response;
                    response.header.id = MsgType::PingServer;
                    
                    response << "Hello from the server!\n"; 
                    client->Send(response);
                }
                break;

                case MsgType::Niga:
                {
                    std::cout << "[" << client->GetID() << "] Server Recieved Message!\n";
                    std::cout << "[" << client->GetID() << "] " << msg.body.data() << "\n";

                    net::message<MsgType> response;
                    response.header.id = MsgType::Niga;
                    
                    response << "Shut UP woman!"; 
                    client->Send(response);
                }
                break;
            }
        }
};


int main ()
{
    CustomServer server(7123);
    server.Start();

    while (2)
    {
        server.Update();
    }

    return 0;
}