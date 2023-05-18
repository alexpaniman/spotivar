#pragma once

#include "backend/session.h"
#include "spotivar-backend.h"

class backend: public sptv::spotivar_backend {
public:
    backend(sptv::spotivar_view &view): view_(view) {}

	void on_folder_selected(std::string root) override;
	void on_search_input(std::string prompt) override;
    // void on_entry_clicked(std::string entry_name) override;

private:
	sptv::spotivar_view &view_;
    tracklist_t currently_shown_tracks_;

    search_result find_tracks_by_prompt(std::string prompt);
	directories::directory_content_obj *get_directiory_content_by_root(std::string root);
};
