#pragma once

#include <string>
#include <vector>


namespace sptv {

    class spotivar_view {
    public:
        virtual void update_entries(std::vector<std::string> entries) = 0;
        virtual void update_folders(std::vector<std::string> folders) = 0;

        virtual ~spotivar_view() = default;
    };

}
