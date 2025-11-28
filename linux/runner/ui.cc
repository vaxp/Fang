#include "ui.h"
#include "tabs.h"
#include "history.h"
#include "bookmarks.h"
#include "adblocker.h"
#include <string.h>
#include <stdio.h>

GtkMenuBar* create_menu_bar(BrowserApp *app) {
  GtkMenuBar *menu_bar = GTK_MENU_BAR(gtk_menu_bar_new());
  
  // File menu
  GtkWidget *file_menu_item = gtk_menu_item_new_with_label("File");
  GtkWidget *file_menu = gtk_menu_new();
  
  GtkWidget *new_tab_item = gtk_menu_item_new_with_label("New Tab");
  g_signal_connect(new_tab_item, "activate", G_CALLBACK(on_new_tab), app);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), new_tab_item);
  
  GtkWidget *close_tab_item = gtk_menu_item_new_with_label("Close Tab");
  g_signal_connect(close_tab_item, "activate", G_CALLBACK(on_close_tab_menu), app);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), close_tab_item);
  
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item), file_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_menu_item);
  
  // View menu
  GtkWidget *view_menu_item = gtk_menu_item_new_with_label("View");
  GtkWidget *view_menu = gtk_menu_new();
  
  GtkWidget *zoom_in_item = gtk_menu_item_new_with_label("Zoom In");
  g_signal_connect(zoom_in_item, "activate", G_CALLBACK(on_zoom_in), app);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), zoom_in_item);
  
  GtkWidget *zoom_out_item = gtk_menu_item_new_with_label("Zoom Out");
  g_signal_connect(zoom_out_item, "activate", G_CALLBACK(on_zoom_out), app);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), zoom_out_item);
  
  GtkWidget *zoom_reset_item = gtk_menu_item_new_with_label("Reset Zoom");
  g_signal_connect(zoom_reset_item, "activate", G_CALLBACK(on_zoom_reset), app);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), zoom_reset_item);
  
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_menu_item), view_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), view_menu_item);
  
  // History menu
  GtkWidget *history_menu_item = gtk_menu_item_new_with_label("History");
  GtkWidget *history_menu = gtk_menu_new();
  
  GtkWidget *show_history_item = gtk_menu_item_new_with_label("Show History");
  g_signal_connect(show_history_item, "activate", G_CALLBACK(show_history_window), app);
  gtk_menu_shell_append(GTK_MENU_SHELL(history_menu), show_history_item);
  
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(history_menu_item), history_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), history_menu_item);
  
  // Bookmarks menu
  GtkWidget *bookmarks_menu_item = gtk_menu_item_new_with_label("Bookmarks");
  GtkWidget *bookmarks_menu = gtk_menu_new();
  
  GtkWidget *add_bookmark_item = gtk_menu_item_new_with_label("Add Bookmark");
  g_signal_connect(add_bookmark_item, "activate", G_CALLBACK(on_add_bookmark), app);
  gtk_menu_shell_append(GTK_MENU_SHELL(bookmarks_menu), add_bookmark_item);
  
  GtkWidget *show_bookmarks_item = gtk_menu_item_new_with_label("Show Bookmarks");
  g_signal_connect(show_bookmarks_item, "activate", G_CALLBACK(show_bookmarks_window), app);
  gtk_menu_shell_append(GTK_MENU_SHELL(bookmarks_menu), show_bookmarks_item);
  
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(bookmarks_menu_item), bookmarks_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), bookmarks_menu_item);
  
  // Privacy menu
  GtkWidget *privacy_menu_item = gtk_menu_item_new_with_label("Privacy");
  GtkWidget *privacy_menu = gtk_menu_new();
  
  GtkWidget *adblock_item = gtk_check_menu_item_new_with_label("Enable AdBlocker");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(adblock_item), app->adblock_enabled);
  g_signal_connect(adblock_item, "toggled", G_CALLBACK(on_adblock_toggled), app);
  gtk_menu_shell_append(GTK_MENU_SHELL(privacy_menu), adblock_item);
  
  GtkWidget *privacy_item = gtk_check_menu_item_new_with_label("Enable Anti-Fingerprinting");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(privacy_item), app->privacy_enabled);
  g_signal_connect(privacy_item, "toggled", G_CALLBACK(on_privacy_toggled), app);
  gtk_menu_shell_append(GTK_MENU_SHELL(privacy_menu), privacy_item);
  
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(privacy_menu_item), privacy_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), privacy_menu_item);
  
  return menu_bar;
}

void activate_uri(BrowserApp *app) {
  if (!app->current_tab) return;
  
  const char *uri = gtk_entry_get_text(app->url_entry);
  if (uri && strlen(uri) > 0) {
    char full_uri[2048];
    if (!strchr(uri, ':')) {
      snprintf(full_uri, sizeof(full_uri), "https://%s", uri);
    } else {
      strncpy(full_uri, uri, sizeof(full_uri) - 1);
      full_uri[sizeof(full_uri) - 1] = '\0';
    }
    webkit_web_view_load_uri(app->current_tab->web_view, full_uri);
  }
}

gboolean force_redraw(BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    gtk_widget_queue_draw(GTK_WIDGET(app->current_tab->web_view));
  }
  return FALSE;
}

void on_back_clicked(GtkButton *button, BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    webkit_web_view_go_back(app->current_tab->web_view);
  }
}

void on_forward_clicked(GtkButton *button, BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    webkit_web_view_go_forward(app->current_tab->web_view);
  }
}

void on_refresh_clicked(GtkButton *button, BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    webkit_web_view_reload(app->current_tab->web_view);
  }
}

void on_home_clicked(GtkButton *button, BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    webkit_web_view_load_uri(app->current_tab->web_view, DEFAULT_HOME);
  }
}

void on_entry_activated(GtkEntry *entry, BrowserApp *app) {
  activate_uri(app);
}

void on_new_tab(GtkMenuItem *item, BrowserApp *app) {
  create_new_tab(app, DEFAULT_HOME);
}

void on_close_tab_menu(GtkMenuItem *item, BrowserApp *app) {
  if (app->current_tab) {
    close_tab(app, app->current_tab);
  }
}

void on_zoom_in(GtkMenuItem *item, BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    app->zoom_level += 0.1;
    if (app->zoom_level > 3.0) app->zoom_level = 3.0;
    webkit_web_view_set_zoom_level(app->current_tab->web_view, app->zoom_level);
  }
}

void on_zoom_out(GtkMenuItem *item, BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    app->zoom_level -= 0.1;
    if (app->zoom_level < 0.25) app->zoom_level = 0.25;
    webkit_web_view_set_zoom_level(app->current_tab->web_view, app->zoom_level);
  }
}

void on_zoom_reset(GtkMenuItem *item, BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    app->zoom_level = 1.0;
    webkit_web_view_set_zoom_level(app->current_tab->web_view, 1.0);
  }
}

gboolean on_window_state_changed(GtkWidget *widget, GdkEventWindowState *event, BrowserApp *app) {
  g_idle_add((GSourceFunc)force_redraw, app);
  return FALSE;
}

void on_adblock_toggled(GtkCheckMenuItem *item, BrowserApp *app) {
  gboolean active = gtk_check_menu_item_get_active(item);
  adblocker_enable(app, active);
  if (app->current_tab && app->current_tab->web_view) {
    webkit_web_view_reload(app->current_tab->web_view);
  }
}

void on_privacy_toggled(GtkCheckMenuItem *item, BrowserApp *app) {
  gboolean active = gtk_check_menu_item_get_active(item);
  privacy_enable(app, active);
  if (app->current_tab && app->current_tab->web_view) {
    webkit_web_view_reload(app->current_tab->web_view);
  }
}
