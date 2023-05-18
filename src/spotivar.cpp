#include "gtk/spotivar-gtk-view.h"
#include "axp/exceptions.h"

#include "quick-backend/quick-backend.h"

int main() {

    sptv::spotivar_gtk_view view;
    quick_backend backend("127.0.0.1", 7123, view);

    view.set_backend(backend); // link backend

    return view.run();
}
