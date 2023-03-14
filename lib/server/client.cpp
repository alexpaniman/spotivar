
#include "client.h"
 
int main()
{
    sptv::spotivar_net_client client("127.0.0.1", 7123);

    client.download_track("/Users/aav/Desktop/new_1.flac", 3); //aka id
    client.download_track("/Users/aav/Desktop/new_2.flac", 33);
    client.download_track("/Users/aav/Desktop/new_3.flac", 7); //there is no file with this indx
    client.download_track("/Users/aav/Desktop/new_4.flac", 12);
    client.download_track("/Users/aav/Desktop/new_5.flac", 14);
    client.download_track("/Users/aav/Desktop/new_6.flac", 55); //same as with 7


    printf ("Tracks were recieved!!!\n");
    sleep (5); 

    printf ("Send a track on server....\n");

    client.upload_track("/Users/aav/Desktop/MY_TRACKS/East_of_Eaden.mp3");
    client.upload_track("/Users/aav/Desktop/MY_TRACKS/Ed_Sheeran_1.mp3");
    client.upload_track("/Users/aav/Desktop/MY_TRACKS/Ed_Sheeran_2.mp3");
    client.upload_track("/Users/aav/Desktop/MY_TRACKS/Ed_Sheeran_3.mp3");
    client.upload_track("/Users/aav/Desktop/MY_TRACKS/Ed_Sheeran_4.mp3");
    
    printf ("all tracks were sent\n");

    sleep (5); 

    return 0;
}