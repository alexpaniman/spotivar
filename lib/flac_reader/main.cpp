#include "FLAC_track_info.h"


int main() {

    FLAC_track_info track_info;
    FLAC_read_track_info("/home/varvara/mipt/flac_reader/flac_tests/aesop.flac", &track_info);
    FLAC_print_track_info(&track_info);
    return 0;
}