#pragma once

#include "FLAC/metadata.h"
#include <iostream>

struct FLAC_track_info {
    uint64_t hash;
    std::string title = "unknown title";
    std::string artist = "unknown artist";
    std::string album = "unknown album";
    std::string comment = "empty comment";
    std::string date = "00_00_00";
    std::string genre = "unknown genre";
    std::string album_artist = "unknown album_artist";
    //picture - ?????????
};


void FLAC_read_track_info(const char *path_to_file, FLAC_track_info *track_info);
void FLAC_print_track_info(FLAC_track_info *track_info);