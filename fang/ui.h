#ifndef UI_H
#define UI_H

#include "types.h"

GtkMenuBar* create_menu_bar(BrowserApp *app);
void activate_uri(BrowserApp *app);
gboolean force_redraw(BrowserApp *app);

// Callbacks
void on_back_clicked(GtkButton *button, BrowserApp *app);
void on_forward_clicked(GtkButton *button, BrowserApp *app);
void on_refresh_clicked(GtkButton *button, BrowserApp *app);
void on_home_clicked(GtkButton *button, BrowserApp *app);
void on_entry_activated(GtkEntry *entry, BrowserApp *app);
void on_new_tab(GtkMenuItem *item, BrowserApp *app);
void on_close_tab_menu(GtkMenuItem *item, BrowserApp *app);
void on_zoom_in(GtkMenuItem *item, BrowserApp *app);
void on_zoom_out(GtkMenuItem *item, BrowserApp *app);
void on_zoom_reset(GtkMenuItem *item, BrowserApp *app);
gboolean on_window_state_changed(GtkWidget *widget, GdkEventWindowState *event, BrowserApp *app);

// Privacy callbacks
void on_adblock_toggled(GtkCheckMenuItem *item, BrowserApp *app);
void on_privacy_toggled(GtkCheckMenuItem *item, BrowserApp *app);

#endif // UI_H
