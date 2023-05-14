#pragma once

#include <string>
#include <vector>
#include "../audio_data_decoder/audio_data.h"
#include "search_results.h"
namespace sptv {

    class spotivar_view {
    public:
        virtual void update_entries(search_result* entries) = 0;
        virtual void update_folders(std::vector<std::string> folders) = 0;

        virtual ~spotivar_view() = default;
    };

}
