#include "server.h"
#include <sys/stat.h>
#include <errno.h>
#include "flac-reader/FLAC_track_info.h"


int main ()
{
    //it binds server to particular "lib" directory//
    sptv::spotivar_server server("/home/alex/test/indexed/", 7123); 
    server.index_track_library();


    while (true) {
        server.operate_user_requests();
    }

    return 0;
}
