#ifndef LGRED_COMPAT_H
#define LGRED_COMPAT_H

#include <gtk/gtk.h>

/* Prototype for compatibility shim implemented in compat.c */
GtkAccelGroup *gtk_menu_ensure_uline_accel_group(GtkMenu *menu);

#endif /* LGRED_COMPAT_H */
