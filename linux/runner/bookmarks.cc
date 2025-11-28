#include "bookmarks.h"
#include <stdio.h>
#include <string.h>

void show_bookmarks_window(GtkMenuItem *item, BrowserApp *app) {
  (void)item; // Unused parameter
  if (!app) {
    g_warning("show_bookmarks_window: app is NULL");
    return;
  }
  
  GtkWindow *parent = NULL;
  if (app->main_window && GTK_IS_WINDOW(app->main_window)) {
    parent = GTK_WINDOW(app->main_window);
  }
  
  GtkWidget *dialog = gtk_dialog_new_with_buttons(
    "Bookmarks",
    parent,
    (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
    "Add Bookmark",
    GTK_RESPONSE_APPLY,
    "Close",
    GTK_RESPONSE_CLOSE,
    NULL
  );
  
  GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled), 400);
  
  GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
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
  gtk_tree_view_column_set_min_width(col2, 400);
  gtk_tree_view_append_column(tree_view, col2);
  
  // Load bookmarks
  gint row_count = 0;
  if (app->bookmarks_db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT url, title FROM bookmarks ORDER BY id DESC";
    if (sqlite3_prepare_v2(app->bookmarks_db, sql, -1, &stmt, NULL) == SQLITE_OK) {
      while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *url = (const char *)sqlite3_column_text(stmt, 0);
        const char *title = (const char *)sqlite3_column_text(stmt, 1);
        
        if (!url) url = "";
        if (!title) title = url;
        
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
          0, title,
          1, url,
          -1);
        row_count++;
      }
      sqlite3_finalize(stmt);
    }
  }
  
  // Show message if no bookmarks
  if (row_count == 0) {
    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
      0, "No bookmarks yet",
      1, "Add bookmarks using Bookmarks → Add Bookmark",
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
  
  // Enable the "Add Bookmark" button
  GtkWidget *add_button = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_APPLY);
  if (add_button) {
    gboolean can_add = (app->current_tab && app->current_tab->web_view && 
                        WEBKIT_IS_WEB_VIEW(app->current_tab->web_view));
    if (can_add) {
      const gchar *url = webkit_web_view_get_uri(app->current_tab->web_view);
      can_add = (url && strlen(url) > 0 && strncmp(url, "about:", 6) != 0);
    }
    gtk_widget_set_sensitive(add_button, can_add);
  }
  
  // Set dialog size
  gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 500);
  
  g_signal_connect(tree_view, "row-activated", G_CALLBACK(on_bookmark_row_activated), app);
  
  // Show and present the dialog
  gtk_widget_show_all(dialog);
  gtk_window_present(GTK_WINDOW(dialog));
  
  gint response = gtk_dialog_run(GTK_DIALOG(dialog));
  if (response == GTK_RESPONSE_APPLY) {
    // Add current page as bookmark
    if (app->current_tab && app->current_tab->web_view && WEBKIT_IS_WEB_VIEW(app->current_tab->web_view)) {
      const gchar *url = webkit_web_view_get_uri(app->current_tab->web_view);
      const gchar *title = webkit_web_view_get_title(app->current_tab->web_view);
      if (url && strlen(url) > 0 && strncmp(url, "about:", 6) != 0 && app->bookmarks_db) {
        // Check if bookmark already exists
        sqlite3_stmt *check_stmt;
        const char *check_sql = "SELECT id FROM bookmarks WHERE url = ?";
        gboolean exists = FALSE;
        if (sqlite3_prepare_v2(app->bookmarks_db, check_sql, -1, &check_stmt, NULL) == SQLITE_OK) {
          sqlite3_bind_text(check_stmt, 1, url, -1, SQLITE_STATIC);
          if (sqlite3_step(check_stmt) == SQLITE_ROW) {
            exists = TRUE;
          }
          sqlite3_finalize(check_stmt);
        }
        
        if (!exists) {
          char *err_msg = NULL;
          char *sql = sqlite3_mprintf(
            "INSERT INTO bookmarks (url, title) VALUES (%Q, %Q);",
            url, title ? title : url
          );
          if (sqlite3_exec(app->bookmarks_db, sql, NULL, NULL, &err_msg) != SQLITE_OK) {
            GtkWidget *error_dialog = gtk_message_dialog_new(
              GTK_IS_WINDOW(app->main_window) ? GTK_WINDOW(app->main_window) : NULL,
              GTK_DIALOG_MODAL,
              GTK_MESSAGE_ERROR,
              GTK_BUTTONS_OK,
              "Failed to add bookmark: %s",
              err_msg
            );
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            sqlite3_free(err_msg);
          } else {
            GtkWidget *info_dialog = gtk_message_dialog_new(
              GTK_IS_WINDOW(app->main_window) ? GTK_WINDOW(app->main_window) : NULL,
              GTK_DIALOG_MODAL,
              GTK_MESSAGE_INFO,
              GTK_BUTTONS_OK,
              "Bookmark added successfully"
            );
            gtk_dialog_run(GTK_DIALOG(info_dialog));
            gtk_widget_destroy(info_dialog);
          }
          sqlite3_free(sql);
        } else {
          GtkWidget *info_dialog = gtk_message_dialog_new(
            GTK_IS_WINDOW(app->main_window) ? GTK_WINDOW(app->main_window) : NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "Bookmark already exists"
          );
          gtk_dialog_run(GTK_DIALOG(info_dialog));
          gtk_widget_destroy(info_dialog);
        }
      }
    }
  }
  
  gtk_widget_destroy(dialog);
}

void on_bookmark_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, BrowserApp *app) {
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

void on_add_bookmark(GtkMenuItem *item, BrowserApp *app) {
  if (!app || !app->current_tab || !app->current_tab->web_view || !WEBKIT_IS_WEB_VIEW(app->current_tab->web_view)) {
    GtkWidget *error_dialog = gtk_message_dialog_new(
      GTK_IS_WINDOW(app->main_window) ? GTK_WINDOW(app->main_window) : NULL,
      GTK_DIALOG_MODAL,
      GTK_MESSAGE_WARNING,
      GTK_BUTTONS_OK,
      "No page available to bookmark"
    );
    gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);
    return;
  }
  
  const gchar *url = webkit_web_view_get_uri(app->current_tab->web_view);
  const gchar *title = webkit_web_view_get_title(app->current_tab->web_view);
  if (!url || strlen(url) == 0 || strncmp(url, "about:", 6) == 0) {
    GtkWidget *error_dialog = gtk_message_dialog_new(
      GTK_IS_WINDOW(app->main_window) ? GTK_WINDOW(app->main_window) : NULL,
      GTK_DIALOG_MODAL,
      GTK_MESSAGE_WARNING,
      GTK_BUTTONS_OK,
      "Cannot bookmark this page"
    );
    gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);
    return;
  }
  
  if (!app->bookmarks_db) {
    GtkWidget *error_dialog = gtk_message_dialog_new(
      GTK_IS_WINDOW(app->main_window) ? GTK_WINDOW(app->main_window) : NULL,
      GTK_DIALOG_MODAL,
      GTK_MESSAGE_ERROR,
      GTK_BUTTONS_OK,
      "Bookmarks database not available"
    );
    gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);
    return;
  }
  
  // Check if bookmark already exists
  sqlite3_stmt *check_stmt;
  const char *check_sql = "SELECT id FROM bookmarks WHERE url = ?";
  gboolean exists = FALSE;
  if (sqlite3_prepare_v2(app->bookmarks_db, check_sql, -1, &check_stmt, NULL) == SQLITE_OK) {
    sqlite3_bind_text(check_stmt, 1, url, -1, SQLITE_STATIC);
    if (sqlite3_step(check_stmt) == SQLITE_ROW) {
      exists = TRUE;
    }
    sqlite3_finalize(check_stmt);
  }
  
  if (exists) {
    GtkWidget *info_dialog = gtk_message_dialog_new(
      GTK_IS_WINDOW(app->main_window) ? GTK_WINDOW(app->main_window) : NULL,
      GTK_DIALOG_MODAL,
      GTK_MESSAGE_INFO,
      GTK_BUTTONS_OK,
      "Bookmark already exists"
    );
    gtk_dialog_run(GTK_DIALOG(info_dialog));
    gtk_widget_destroy(info_dialog);
    return;
  }
  
  char *err_msg = NULL;
  char *sql = sqlite3_mprintf(
    "INSERT INTO bookmarks (url, title) VALUES (%Q, %Q);",
    url, title ? title : url
  );
  if (sqlite3_exec(app->bookmarks_db, sql, NULL, NULL, &err_msg) != SQLITE_OK) {
    GtkWidget *error_dialog = gtk_message_dialog_new(
      GTK_IS_WINDOW(app->main_window) ? GTK_WINDOW(app->main_window) : NULL,
      GTK_DIALOG_MODAL,
      GTK_MESSAGE_ERROR,
      GTK_BUTTONS_OK,
      "Failed to add bookmark: %s",
      err_msg
    );
    gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);
    sqlite3_free(err_msg);
  } else {
    GtkWidget *info_dialog = gtk_message_dialog_new(
      GTK_IS_WINDOW(app->main_window) ? GTK_WINDOW(app->main_window) : NULL,
      GTK_DIALOG_MODAL,
      GTK_MESSAGE_INFO,
      GTK_BUTTONS_OK,
      "Bookmark added successfully"
    );
    gtk_dialog_run(GTK_DIALOG(info_dialog));
    gtk_widget_destroy(info_dialog);
  }
  sqlite3_free(sql);
}
