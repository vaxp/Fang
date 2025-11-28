#include "types.h"
#include "database.h"
#include "tabs.h"
#include "ui.h"
#include "adblocker.h"
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);

  if (!webkit_web_context_get_default()) {
    g_error("Failed to get WebKit context");
    return 1;
  }

  BrowserApp *app = g_new0(BrowserApp, 1);
  app->next_tab_id = 1;
  app->zoom_level = 1.0;
  
  // Configure persistent storage
  app->web_context = webkit_web_context_get_default();
  setup_persistent_storage(app->web_context);
  
  // Connect download handler
  g_signal_connect(app->web_context, "download-started", G_CALLBACK(on_download_started), app);
  
  // Initialize databases
  initialize_databases(app);
  
  // Initialize AdBlocker
  adblocker_init(app);

  // Create main window
  app->main_window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_default_size(app->main_window, 1200, 720);
  gtk_window_set_title(app->main_window, BROWSER_NAME);
  g_signal_connect(app->main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(app->main_window, "window-state-event", G_CALLBACK(on_window_state_changed), app);

  // Create main vertical box
  GtkBox *vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  
  // Create menu bar
  app->menu_bar = create_menu_bar(app);
  gtk_box_pack_start(vbox, GTK_WIDGET(app->menu_bar), FALSE, FALSE, 0);

  // Create toolbar
  GtkBox *toolbar = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5));
  gtk_container_set_border_width(GTK_CONTAINER(toolbar), 5);

  // Navigation buttons
  app->back_button = GTK_BUTTON(gtk_button_new_from_icon_name("go-previous", GTK_ICON_SIZE_BUTTON));
  g_signal_connect(app->back_button, "clicked", G_CALLBACK(on_back_clicked), app);
  gtk_box_pack_start(toolbar, GTK_WIDGET(app->back_button), FALSE, FALSE, 0);

  app->forward_button = GTK_BUTTON(gtk_button_new_from_icon_name("go-next", GTK_ICON_SIZE_BUTTON));
  g_signal_connect(app->forward_button, "clicked", G_CALLBACK(on_forward_clicked), app);
  gtk_box_pack_start(toolbar, GTK_WIDGET(app->forward_button), FALSE, FALSE, 0);

  app->refresh_button = GTK_BUTTON(gtk_button_new_from_icon_name("view-refresh", GTK_ICON_SIZE_BUTTON));
  g_signal_connect(app->refresh_button, "clicked", G_CALLBACK(on_refresh_clicked), app);
  gtk_box_pack_start(toolbar, GTK_WIDGET(app->refresh_button), FALSE, FALSE, 0);
  
  app->home_button = GTK_BUTTON(gtk_button_new_from_icon_name("go-home", GTK_ICON_SIZE_BUTTON));
  g_signal_connect(app->home_button, "clicked", G_CALLBACK(on_home_clicked), app);
  gtk_box_pack_start(toolbar, GTK_WIDGET(app->home_button), FALSE, FALSE, 0);
  
  // New Tab button
  GtkButton *new_tab_button = GTK_BUTTON(gtk_button_new_from_icon_name("tab-new", GTK_ICON_SIZE_BUTTON));
  g_signal_connect(new_tab_button, "clicked", G_CALLBACK(on_new_tab), app);
  gtk_box_pack_start(toolbar, GTK_WIDGET(new_tab_button), FALSE, FALSE, 0);

  // Separator
  GtkSeparator *sep = GTK_SEPARATOR(gtk_separator_new(GTK_ORIENTATION_VERTICAL));
  gtk_box_pack_start(toolbar, GTK_WIDGET(sep), FALSE, FALSE, 0);

  // URL entry
  app->url_entry = GTK_ENTRY(gtk_entry_new());
  gtk_entry_set_text(app->url_entry, DEFAULT_HOME);
  g_signal_connect(app->url_entry, "activate", G_CALLBACK(on_entry_activated), app);
  gtk_box_pack_start(toolbar, GTK_WIDGET(app->url_entry), TRUE, TRUE, 0);

  // Go button
  GtkButton *go_button = GTK_BUTTON(gtk_button_new_with_label("Go"));
  g_signal_connect(go_button, "clicked", G_CALLBACK(activate_uri), app);
  gtk_box_pack_start(toolbar, GTK_WIDGET(go_button), FALSE, FALSE, 0);

  gtk_box_pack_start(vbox, GTK_WIDGET(toolbar), FALSE, FALSE, 0);

  // Create notebook for tabs
  app->notebook = GTK_NOTEBOOK(gtk_notebook_new());
  gtk_notebook_set_scrollable(app->notebook, TRUE);
  gtk_notebook_popup_enable(app->notebook);
  gtk_notebook_set_show_tabs(app->notebook, TRUE);
  gtk_notebook_set_show_border(app->notebook, TRUE);
  g_signal_connect(app->notebook, "switch-page", G_CALLBACK(on_tab_switched), app);
  gtk_box_pack_start(vbox, GTK_WIDGET(app->notebook), TRUE, TRUE, 0);

  // Add vbox to window
  gtk_container_add(GTK_CONTAINER(app->main_window), GTK_WIDGET(vbox));

  // Create initial tab
  create_new_tab(app, DEFAULT_HOME);

  // Show all
  gtk_widget_show_all(GTK_WIDGET(app->main_window));

  gtk_main();
  
  // Cleanup
  if (app->history_db) {
    sqlite3_close(app->history_db);
  }
  if (app->bookmarks_db) {
    sqlite3_close(app->bookmarks_db);
  }

  return 0;
}
