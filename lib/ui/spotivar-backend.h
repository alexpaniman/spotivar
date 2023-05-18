#pragma once

#include <string>
#include <vector>

#include "spotivar-view.h"

#include "audio-data-decoder/audio_data.h"

#include "backend/session.h"
#include "backend/search_results.h"
#include "backend/directories.h"

namespace sptv {

    class spotivar_backend {
    public:
		virtual void on_search_input(std::string updated_prompt) = 0;
		virtual void on_folder_selected(std::string selected_folder) = 0;
		virtual void on_entry_clicked(int entry_id) = 0;

		virtual ~spotivar_backend() = default;
	};

}
