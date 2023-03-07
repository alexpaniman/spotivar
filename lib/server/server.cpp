#include "server.h"
#include "Text.h"
#include <sys/stat.h>
#include <errno.h>

typedef struct stat my_stat;

enum class MsgType : uint32_t
{
    Connect,
    Message,
    PingServer,
    Niga,
    SendTrack
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

                case MsgType::SendTrack:
                {
                    FILE* mp3 = fopen("Ya_Pizdatiy.mp3", "rb"); //WHAT?? WHY DOES IT RETURN NULL??

                    if (!mp3) {
                        perror("fopen ");
                        /// die
                    }

                    printf ("the file address %p\n", mp3);
                    //printf ("errno code is %d\n", errno);

                    long long int file_size = GetFileSize(mp3);
                    printf ("file size cheeeck: %llu\n", file_size);

                    uint8_t* TempBuffer = (uint8_t*)calloc(file_size, sizeof(uint8_t));
                    fread (TempBuffer, sizeof(uint8_t), file_size, mp3);

                    printf ("Song in a ram\n"); 

                    net::message<MsgType> Track;
                    Track.header.id = MsgType::SendTrack;

                    Track.body.resize(file_size);
                    memcpy (Track.body.data(), TempBuffer, file_size);

                    printf ("It was fully copied into message\n");

                    Track.header.size = Track.size(); //update message size to send

                    client->Send(Track);

                    free  (TempBuffer);
                    fclose(mp3);

                    //     my_stat buf = {}; it is fucking mp3 though
                    //     stat ("./Ya_Pizdatiy.mp3", &buf);

                    //     printf ("file block size %llu\n", buf.st_blksize);
                    //     printf ("file size is    %llu\n", buf.st_size);
                    // }
                }
                break;

            }
        }
};


int main ()
{
    CustomServer server(7123);
    server.Start();

    while (true)
    {
        server.Update();

    }

    return 0;
}