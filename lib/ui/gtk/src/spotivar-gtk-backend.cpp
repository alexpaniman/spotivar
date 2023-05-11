#include "gtk/spotivar-gtk-backend.h"
#include "gtk/spotivar-glade-ui.h"

#include "gtkmm.h"
#include "fmt/format.h"
#include <memory>


namespace sptv {

    struct spotivar_gtk_backend::impl {
        Glib::RefPtr<Gtk::Application> app;
        std::unique_ptr<Gtk::Window> window;
    };


    spotivar_gtk_backend::spotivar_gtk_backend() {
        auto build = Gtk::Builder::create_from_string(sptv::spotivar_glade_ui);
        impl_->window.reset(build->get_widget<Gtk::Window>("main-window"));

        impl_->window->show();
    }


    void spotivar_gtk_backend::on_search_input(std::string updated_prompt) {

    }

    void spotivar_gtk_backend::on_folder_selected(std::string selected_folder) {

    }

    void spotivar_gtk_backend::on_entry_clicked(std::string entry_name) {

    }

};

