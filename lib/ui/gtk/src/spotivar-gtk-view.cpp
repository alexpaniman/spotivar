#include "gtk/spotivar-gtk-view.h"
#include "gtk/spotivar-glade-ui.h"

#include "gtkmm.h"
#include "fmt/format.h"
#include "gtkmm/application.h"
#include <memory>
#include "../../search_results.h"


namespace sptv {

    struct spotivar_gtk_view::impl {
        Glib::RefPtr<Gtk::Application> app;
        Gtk::Window *window;
    };

    void spotivar_gtk_view::impl_deleter::operator()(impl* impl) const {
        // just delete pointer, needed because std::unique_pointer<>
        // in some implementations statically asserts sizeof of type
        // is bigger than 0, and that doesn't work on incomplete ts

        delete impl;
    }


    spotivar_gtk_view::spotivar_gtk_view() {
        
        std::unique_ptr<impl, spotivar_gtk_view::impl_deleter> impl(new spotivar_gtk_view::impl());

        impl_.swap(impl);
        impl_->app = Gtk::Application::create("org.gtkmm.example.base");

        impl_->app->signal_startup().connect([&] {
            auto build = Gtk::Builder::create_from_string(sptv::spotivar_glade_ui);

            build->get_widget<Gtk::Window>("main-window", impl_->window);
            impl_->window->set_application(impl_->app);

            impl_->window->show();
        });
    }

    void spotivar_gtk_view::update_entries(search_result *entries) {
        (void) entries;
        // TODO: ...
    }

    void spotivar_gtk_view::update_folders(std::vector<std::string> folders) {
        (void) folders;
        // TODO: ...
    }


    int spotivar_gtk_view::run() {
        return impl_->app->run();
    }

};

