#include "server.h"
#include <sys/stat.h>
#include <errno.h>


int main ()
{
    sptv::spotivar_server server("/Users/aav/Desktop/TEST_BASE/", 7123); //it binds server to particular "lib" directory
    server.indexing_server_library_on_start();

    while (true) {
        server.update();
    }

    return 0;
}
