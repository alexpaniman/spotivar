#pragma once

#include <string>
#include <vector>
#include "../audio_data_decoder/audio_data.h"
#include "search_results.h"
#include "directories.h"
namespace sptv {

    class spotivar_view {
    public:
        virtual void update_entries(search_result*) = 0;
        virtual void update_folders(directories::directory_content_t*) = 0;

        virtual ~spotivar_view() = default;
    };

}
