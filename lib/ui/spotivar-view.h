#pragma once

#include <string>
#include <vector>

#include "backend/search_results.h"
#include "directory-scanner/directories.h"

namespace sptv {

    class spotivar_view {
    public:
        virtual void update_entries(search_result*) = 0;
        virtual void update_folders(directories::directory_content_obj*) = 0;

        virtual ~spotivar_view() = default;
    };
}
