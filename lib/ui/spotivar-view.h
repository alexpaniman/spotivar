#pragma once

#include <string>
#include <vector>

#include "backend/search_results.h"
#include "directory-scanner/directories.h"

namespace sptv {

    class spotivar_view {
    public:
        virtual void update_entries(std::vector<std::string> entries) = 0;
        virtual void update_folders(std::vector<std::string> folders) = 0;

        virtual ~spotivar_view() = default;
    };
}
