#include "quick-backend/quick-backend.h"
#include "fmt/format.h"


quick_backend::quick_backend(const std::string host, uint16_t port,
                             sptv::spotivar_view &view):
    client_(host, port), view_(view) {

    tracks_ = client_.get_tracks_list();

    std::vector<std::string> stringified;
    for (auto &&track: tracks_)
        stringified.push_back(std::move(track.title));

    view_.update_entries(stringified);
}
		
// void quick_backend::on_search_input(std::string updated_prompt) {
//     // TODO: ...
// }

// void quick_backend::on_folder_selected(std::string selected_folder) {}

void quick_backend::on_entry_clicked(int entry_index) {
    fmt::print("You clicked track: {}", tracks_[entry_index].title);
}
