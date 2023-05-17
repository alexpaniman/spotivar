#pragma once

#include "spotivar-view.h"

#include <memory>
#include <string>
#include <vector>
#include "backend/search_results.h"
#include "directory_scaner/directories.h"


namespace sptv {
    
    class spotivar_gtk_view: public sptv::spotivar_view {
    public:
        spotivar_gtk_view();

        void update_entries(search_result *entries) override;
        void update_folders(directories::directory_content_t *folders) override;

        int run(); // start ui even loop

        ~spotivar_gtk_view() = default;

    private:
        struct impl; // forward declare gtk implementation details
        struct impl_deleter { void operator()(impl*) const; };

        // use pimpl idiom to prevent gtk headers from leaking outside:
        std::unique_ptr<impl, impl_deleter> impl_;             
    };

}
