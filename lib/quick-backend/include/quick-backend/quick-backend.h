#pragma once

#include "spotivar-backend.h"
#include "spotivar-view.h"
#include "client.h"


class quick_backend: public sptv::spotivar_backend {
public:
    quick_backend(const std::string host, uint16_t port,
                  sptv::spotivar_view &view);

    void on_search_input(std::string updated_prompt) override;
    void on_folder_selected(std::string selected_folder) override;
    void on_entry_clicked(int entry_index) override;

private:
    sptv::spotivar_net_client client_;
    sptv::spotivar_view &view_;

    std::vector<FLAC_track_info> tracks_;
};
