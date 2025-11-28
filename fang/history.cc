#include "history.h"
#include <stdio.h>
#include <string.h>

void show_history_window(GtkMenuItem *item, BrowserApp *app) {
  (void)item; // Unused parameter
  if (!app) {
    g_warning("show_history_window: app is NULL");
    return;
  }
  
  GtkWindow *parent = NULL;
  if (app->main_window && GTK_IS_WINDOW(app->main_window)) {
    parent = GTK_WINDOW(app->main_window);
  }
  
  GtkWidget *dialog = gtk_dialog_new_with_buttons(
    "History",
    parent,
    (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
    "Clear History",
    GTK_RESPONSE_APPLY,
    "Close",
    GTK_RESPONSE_CLOSE,
    NULL
  );
  
  GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled), 400);
  
  GtkListStore *store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  GtkTreeView *tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(store)));
  
  GtkCellRenderer *renderer1 = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *col1 = gtk_tree_view_column_new_with_attributes("Title", renderer1, "text", 0, NULL);
  gtk_tree_view_column_set_resizable(col1, TRUE);
  gtk_tree_view_column_set_min_width(col1, 200);
  gtk_tree_view_append_column(tree_view, col1);
  
  GtkCellRenderer *renderer2 = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *col2 = gtk_tree_view_column_new_with_attributes("URL", renderer2, "text", 1, NULL);
  gtk_tree_view_column_set_resizable(col2, TRUE);
  gtk_tree_view_column_set_expand(col2, TRUE);
  gtk_tree_view_column_set_min_width(col2, 300);
  gtk_tree_view_append_column(tree_view, col2);
  
  GtkCellRenderer *renderer3 = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *col3 = gtk_tree_view_column_new_with_attributes("Date", renderer3, "text", 2, NULL);
  gtk_tree_view_column_set_resizable(col3, TRUE);
  gtk_tree_view_column_set_min_width(col3, 150);
  gtk_tree_view_append_column(tree_view, col3);
  
  // Load history
  gint row_count = 0;
  if (app->history_db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT url, title, visit_time FROM history ORDER BY visit_time DESC LIMIT 500";
    if (sqlite3_prepare_v2(app->history_db, sql, -1, &stmt, NULL) == SQLITE_OK) {
      while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *url = (const char *)sqlite3_column_text(stmt, 0);
        const char *title = (const char *)sqlite3_column_text(stmt, 1);
        time_t visit_time = sqlite3_column_int64(stmt, 2);
        
        if (!url) url = "";
        if (!title) title = url;
        
        char time_str[64] = "";
        if (visit_time > 0) {
          struct tm *tm_info = localtime(&visit_time);
          if (tm_info) {
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", tm_info);
          }
        }
        
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
          0, title,
          1, url,
          2, time_str,
          -1);
        row_count++;
      }
      sqlite3_finalize(stmt);
    }
  }
  
  // Show message if no history
  if (row_count == 0) {
    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
      0, "No history yet",
      1, "Start browsing to build your history",
      2, "",
      -1);
  }
  
  // Set tree view properties
  gtk_tree_view_set_headers_visible(tree_view, TRUE);
  gtk_tree_view_set_rules_hint(tree_view, TRUE);
  
  gtk_container_add(GTK_CONTAINER(scrolled), GTK_WIDGET(tree_view));
  gtk_container_add(GTK_CONTAINER(content), scrolled);
  
  // Show tree view explicitly
  gtk_widget_show(GTK_WIDGET(tree_view));
  gtk_widget_show(scrolled);
  gtk_widget_show(content);
  
  // Set dialog size
  gtk_window_set_default_size(GTK_WINDOW(dialog), 800, 500);
  
  // Handle double-click to navigate
  g_signal_connect(tree_view, "row-activated", G_CALLBACK(on_history_row_activated), app);
  
  // Show and present the dialog
  gtk_widget_show_all(dialog);
  gtk_window_present(GTK_WINDOW(dialog));
  
  gint response = gtk_dialog_run(GTK_DIALOG(dialog));
  if (response == GTK_RESPONSE_APPLY) {
    // Clear history
    if (app->history_db) {
      char *err_msg = NULL;
      if (sqlite3_exec(app->history_db, "DELETE FROM history", NULL, NULL, &err_msg) != SQLITE_OK) {
        GtkWidget *error_dialog = gtk_message_dialog_new(
          GTK_IS_WINDOW(app->main_window) ? GTK_WINDOW(app->main_window) : NULL,
          GTK_DIALOG_MODAL,
          GTK_MESSAGE_ERROR,
          GTK_BUTTONS_OK,
          "Failed to clear history: %s",
          err_msg
        );
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        sqlite3_free(err_msg);
      }
    }
  }
  
  gtk_widget_destroy(dialog);
}

void on_history_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, BrowserApp *app) {
  if (!app || !app->current_tab || !app->current_tab->web_view) return;
  
  GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
  if (!model) return;
  
  GtkTreeIter iter;
  if (gtk_tree_model_get_iter(model, &iter, path)) {
    gchar *url = NULL;
    gtk_tree_model_get(model, &iter, 1, &url, -1);
    if (url && strlen(url) > 0) {
      webkit_web_view_load_uri(app->current_tab->web_view, url);
    }
    if (url) {
      g_free(url);
    }
  }
}
