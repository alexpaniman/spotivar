#include "gtk/spotivar-gtk-view.h"
#include "gtk/spotivar-glade-ui.h"

// TODO: WTF dependency on backend, shouldn't be
#include "backend/search_results.h"

#include "fmt/format.h"

#include "gtkmm.h"
#include "gtkmm/treeviewcolumn.h"
#include "sigc++/functors/mem_fun.h"
#include <memory>


namespace sptv {

    class column_model : public Gtk::TreeModel::ColumnRecord {
    public:
        column_model() { add(column_name); }

        Gtk::TreeModelColumn<Glib::ustring> column_name;
    };

    struct spotivar_gtk_view::impl {
        Glib::RefPtr<Gtk::Application> app;
        std::unique_ptr<Gtk::Window> window;

        std::unique_ptr<Gtk::TreeView> folders;
        std::unique_ptr<Gtk::TreeView> entries;
        std::unique_ptr<Gtk::Button> play;

        column_model model;
        Glib::RefPtr<Gtk::ListStore> entries_store_model;
        Glib::RefPtr<Gtk::ListStore> folders_store_model;
    };

    void spotivar_gtk_view::impl_deleter::operator()(impl* impl) const {
        // just delete pointer, needed because std::unique_pointer<>
        // in some implementations statically asserts sizeof of type
        // is bigger than 0, and that doesn't work on incomplete ts

        delete impl;
    }


    template <typename widget_type>
    static widget_type* load_widget(std::unique_ptr<widget_type> &widget,
            Glib::RefPtr<Gtk::Builder> builder, const char *name) {

        widget_type *loaded_widget = nullptr;
        builder->get_widget(name, loaded_widget);
        widget.reset(loaded_widget);

        return widget.get();
    }

    spotivar_gtk_view::spotivar_gtk_view() {
        
        std::unique_ptr<impl, spotivar_gtk_view::impl_deleter> impl(new spotivar_gtk_view::impl());

        impl_.swap(impl);
        impl_->app = Gtk::Application::create("org.gtkmm.example.base");

        impl_->app->signal_startup().connect([&] {
            auto build = Gtk::Builder::create_from_string(sptv::spotivar_glade_ui);

            Gtk::Window *window = load_widget(impl_->window, build, "main-window");
            load_widget(impl_->folders, build, "folders");
            load_widget(impl_->entries, build, "entries");
            load_widget(impl_->play, build, "play");

            window->set_application(impl_->app);
            window->show();

            impl_->folders_store_model = Gtk::ListStore::create(impl_->model);
            impl_->folders->set_model(impl_->folders_store_model);
            impl_->folders->set_headers_visible(false);
            impl_->folders->append_column("Name", impl_->model.column_name);

            impl_->entries_store_model = Gtk::ListStore::create(impl_->model);
            impl_->entries->set_model(impl_->entries_store_model);
            impl_->entries->set_headers_visible(false);
            impl_->entries->append_column("Name", impl_->model.column_name);

            impl_->entries->signal_row_activated().connect([&](const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *column) {
                backend_->on_entry_clicked(path.front());
            });

            backend_->init();
        });
    }

    // TODO: remove duplication
    void spotivar_gtk_view::update_entries(std::vector<std::string> entries) {
        impl_->entries_store_model->clear();
        for (auto &&entry: entries) {
            auto row = *(impl_->entries_store_model->append());
            row[impl_->model.column_name] = entry;
        }
    }


    void spotivar_gtk_view::update_folders(std::vector<std::string> folders) {
        impl_->folders_store_model->clear();
        for (auto &&entry: folders) {
            auto row = *(impl_->folders_store_model->append());
            row[impl_->model.column_name] = entry;
        }
    }


    int spotivar_gtk_view::run() {
        return impl_->app->run();
    }

};

