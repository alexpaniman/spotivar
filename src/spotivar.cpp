#include "gtk/spotivar-gtk-view.h"
#include "axp/exceptions.h"

#include "quick-backend/quick-backend.h"

int main() {

    sptv::spotivar_gtk_view view;
    quick_backend backend("host", 123, view);

    view.set_backend(backend); // link backend

    return view.run();
}
