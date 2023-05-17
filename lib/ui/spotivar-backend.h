#pragma once

#include <string>
#include <vector>
#include "spotivar-view.h"
#include "../audio_data_decoder/audio_data.h"
#include "session.h"
#include "search_results.h"
#include "directories.h"

namespace sptv {

    class spotivar_backend {
    public:
		spotivar_backend(session &current_session):
			session_(current_session) {}
		
		virtual void on_search_input(std::string updated_prompt) = 0;
		virtual void on_folder_selected(std::string selected_folder) = 0;
		virtual void on_entry_clicked(std::string entry_name) = 0;

		virtual ~spotivar_backend() = default;
	protected:
		session &session_;
	};
}


class backend: public sptv::spotivar_backend {
public:
    backend(session &current_session, sptv::spotivar_view &view):
        spotivar_backend(current_session), view_(view) {}

	search_result *find_tracks_by_prompt(std::string prompt);

	directories::directory_content_t *get_directiory_content_by_root(std::string root);

	void on_folder_selected(std::string root) override {
		view_.update_folders(get_directiory_content_by_root(root));
	}
	void on_search_input(std::string prompt) override {
		view_.update_entries(find_tracks_by_prompt(prompt));
	}

private:
	sptv::spotivar_view &view_;
};
