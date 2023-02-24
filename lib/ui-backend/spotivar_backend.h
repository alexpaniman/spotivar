#pragma once

#include <string>
#include <vector>

class spotivar_backend {
public:
    virtual void on_search_input(std::string updated_prompt) = 0;
    virtual void on_folder_selected(std::string selected_folder) = 0;
    virtual void on_entry_clicked(std::string entry_name) = 0;

    virtual ~spotivar_backend() = 0;
};
