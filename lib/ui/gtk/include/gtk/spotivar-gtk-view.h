#pragma once

#include "spotivar-view.h"

#include <memory>
#include <string>
#include <vector>
#include "../../search_results.h"

namespace sptv {
    
    class spotivar_gtk_view: public sptv::spotivar_view {
    public:
        spotivar_gtk_view();

        void update_entries(search_result *entries) override;
        void update_folders(std::vector<std::string> folders) override;

        int run(); // start ui even loop

        ~spotivar_gtk_view() = default;

    private:
        struct impl; // forward declare gtk implementation details
        struct impl_deleter { void operator()(impl*) const; };

        // use pimpl idiom to prevent gtk headers from leaking outside:
        std::unique_ptr<impl, impl_deleter> impl_;             
    };

}
