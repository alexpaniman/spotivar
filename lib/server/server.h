#pragma once

#include <string>
#include <vector>

class track {
    std::string title, artist; 
    std::vector<char> data; // Subject to change
};

class music_client {
    music_client();

    std::vector<std::string> get_music_list();
    track recieve_song(std::string title);
};

