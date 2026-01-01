#include <gtk/gtk.h>

GtkAccelGroup *
gtk_menu_ensure_uline_accel_group(GtkMenu *menu)
{
    GtkAccelGroup *g = gtk_accel_group_new();
    if (GTK_IS_MENU(menu))
        gtk_menu_set_accel_group(menu, g);
    return g;
}
