#pragma once

#include <string>
#include <vector>

class song {
    std::string title, artist; 
    std::vector<char> data; // Subject to change
};

class music_client {
    music_client();

    std::vector<std::string> get_music_list();
    song recieve_song(std::string title);
};

