#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <sqlite3.h>
#include <time.h>

#define BROWSER_NAME "Vaxp Browser"
#define DEFAULT_HOME "https://www.google.com"

// Tab structure
typedef struct {
  WebKitWebView *web_view;
  GtkWidget *tab_label;
  GtkWidget *close_button;
  gchar *title;
  gchar *uri;
  gint tab_id;
} BrowserTab;

// History entry
typedef struct {
  gchar *url;
  gchar *title;
  time_t visit_time;
} HistoryEntry;

// Bookmark entry
typedef struct {
  gchar *url;
  gchar *title;
  gchar *folder;
} BookmarkEntry;

// Main application structure
typedef struct {
  GtkWindow *main_window;
  GtkNotebook *notebook;
  GtkEntry *url_entry;
  GtkButton *back_button;
  GtkButton *forward_button;
  GtkButton *refresh_button;
  GtkButton *home_button;
  GtkMenuBar *menu_bar;
  WebKitWebContext *web_context;
  sqlite3 *history_db;
  sqlite3 *bookmarks_db;
  GList *tabs;  // List of BrowserTab*
  BrowserTab *current_tab;
  gint next_tab_id;
  gdouble zoom_level;
} BrowserApp;

// Forward declarations
static BrowserTab* create_new_tab(BrowserApp *app, const gchar *uri);
static void close_tab(BrowserApp *app, BrowserTab *tab);
static void switch_to_tab(BrowserApp *app, BrowserTab *tab);
static void update_url_bar(BrowserApp *app, BrowserTab *tab);
static void add_to_history(BrowserApp *app, const gchar *url, const gchar *title);
static void initialize_databases(BrowserApp *app);
static void setup_persistent_storage(WebKitWebContext *context);
static void on_uri_changed(WebKitWebView *web_view, GParamSpec *spec, BrowserApp *app);
static void on_title_changed(WebKitWebView *web_view, GParamSpec *spec, BrowserApp *app);
static void on_load_changed(WebKitWebView *web_view, WebKitLoadEvent load_event, BrowserApp *app);
static void on_close_tab_clicked(GtkButton *button, BrowserApp *app);
static void on_tab_switched(GtkNotebook *notebook, GtkWidget *page, guint page_num, BrowserApp *app);
static void show_history_window(GtkMenuItem *item, BrowserApp *app);
static void show_bookmarks_window(GtkMenuItem *item, BrowserApp *app);
static void on_history_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, BrowserApp *app);
static void on_bookmark_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, BrowserApp *app);
static gboolean on_decide_policy(WebKitWebView *web_view, WebKitPolicyDecision *decision, WebKitPolicyDecisionType decision_type, BrowserApp *app);
static gboolean on_permission_request(WebKitWebView *web_view, WebKitPermissionRequest *request, BrowserApp *app);
static gboolean on_enter_fullscreen(WebKitWebView *web_view, BrowserApp *app);
static gboolean on_leave_fullscreen(WebKitWebView *web_view, BrowserApp *app);
static void on_download_started(WebKitWebContext *context, WebKitDownload *download, BrowserApp *app);

// Database initialization
static void initialize_databases(BrowserApp *app) {
  const char *home = g_get_home_dir();
  char data_dir[2048];
  char history_file[2048];
  char bookmarks_file[2048];
  
  snprintf(data_dir, sizeof(data_dir), "%s/.local/share/vaxp-browser", home);
  g_mkdir_with_parents(data_dir, 0700);
  
  snprintf(history_file, sizeof(history_file), "%s/history.db", data_dir);
  snprintf(bookmarks_file, sizeof(bookmarks_file), "%s/bookmarks.db", data_dir);
  
  // Initialize history database
  if (sqlite3_open(history_file, &app->history_db) != SQLITE_OK) {
    g_warning("Failed to open history database: %s", sqlite3_errmsg(app->history_db));
    app->history_db = NULL;
  } else {
    char *err_msg = NULL;
    const char *create_history = 
      "CREATE TABLE IF NOT EXISTS history ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "url TEXT NOT NULL,"
      "title TEXT,"
      "visit_time INTEGER NOT NULL"
      ");"
      "CREATE INDEX IF NOT EXISTS idx_visit_time ON history(visit_time DESC);";
    if (sqlite3_exec(app->history_db, create_history, NULL, NULL, &err_msg) != SQLITE_OK) {
      g_warning("Failed to create history table: %s", err_msg);
      sqlite3_free(err_msg);
    }
  }
  
  // Initialize bookmarks database
  if (sqlite3_open(bookmarks_file, &app->bookmarks_db) != SQLITE_OK) {
    g_warning("Failed to open bookmarks database: %s", sqlite3_errmsg(app->bookmarks_db));
    app->bookmarks_db = NULL;
  } else {
    char *err_msg = NULL;
    const char *create_bookmarks = 
      "CREATE TABLE IF NOT EXISTS bookmarks ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "url TEXT NOT NULL,"
      "title TEXT,"
      "folder TEXT DEFAULT 'Other Bookmarks'"
      ");";
    if (sqlite3_exec(app->bookmarks_db, create_bookmarks, NULL, NULL, &err_msg) != SQLITE_OK) {
      g_warning("Failed to create bookmarks table: %s", err_msg);
      sqlite3_free(err_msg);
    }
  }
}

// Add entry to history
static void add_to_history(BrowserApp *app, const gchar *url, const gchar *title) {
  if (!app->history_db || !url) return;
  
  char *err_msg = NULL;
  char *sql = sqlite3_mprintf(
    "INSERT INTO history (url, title, visit_time) VALUES (%Q, %Q, %lld);",
    url, title ? title : "", (long long)time(NULL)
  );
  
  if (sqlite3_exec(app->history_db, sql, NULL, NULL, &err_msg) != SQLITE_OK) {
    g_warning("Failed to add history entry: %s", err_msg);
    sqlite3_free(err_msg);
  }
  
  sqlite3_free(sql);
}

// Create new tab
static BrowserTab* create_new_tab(BrowserApp *app, const gchar *uri) {
  BrowserTab *tab = g_new0(BrowserTab, 1);
  tab->tab_id = app->next_tab_id++;
  tab->title = g_strdup("New Tab");
  tab->uri = g_strdup(uri ? uri : "about:blank");
  
  // Create web view
  WebKitSettings *settings = webkit_settings_new();
  webkit_settings_set_hardware_acceleration_policy(settings, WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS);
  webkit_settings_set_enable_page_cache(settings, TRUE);
  webkit_settings_set_javascript_can_open_windows_automatically(settings, FALSE);
  webkit_settings_set_enable_fullscreen(settings, TRUE);
  webkit_settings_set_enable_media_stream(settings, TRUE);
  webkit_settings_set_enable_encrypted_media(settings, TRUE);
  webkit_settings_set_allow_universal_access_from_file_urls(settings, TRUE);
  webkit_settings_set_allow_file_access_from_file_urls(settings, TRUE);
  
  tab->web_view = WEBKIT_WEB_VIEW(webkit_web_view_new_with_context(app->web_context));
  webkit_web_view_set_settings(tab->web_view, settings);
  
  // Create tab label with close button
  GtkBox *label_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5));
  GtkLabel *label = GTK_LABEL(gtk_label_new("New Tab"));
  tab->close_button = gtk_button_new_from_icon_name("window-close", GTK_ICON_SIZE_MENU);
  gtk_widget_set_tooltip_text(tab->close_button, "Close Tab");
  gtk_widget_set_size_request(tab->close_button, 20, 20);
  
  gtk_box_pack_start(label_box, GTK_WIDGET(label), TRUE, TRUE, 0);
  gtk_box_pack_start(label_box, tab->close_button, FALSE, FALSE, 0);
  gtk_widget_show_all(GTK_WIDGET(label_box));
  
  tab->tab_label = GTK_WIDGET(label_box);
  
  // Store tab pointer in web_view and close button for later retrieval
  g_object_set_data(G_OBJECT(tab->web_view), "tab-data", tab);
  g_object_set_data(G_OBJECT(tab->close_button), "tab-data", tab);
  
  // Connect signals
  g_signal_connect(tab->web_view, "notify::uri", G_CALLBACK(on_uri_changed), app);
  g_signal_connect(tab->web_view, "notify::title", G_CALLBACK(on_title_changed), app);
  g_signal_connect(tab->web_view, "load-changed", G_CALLBACK(on_load_changed), app);
  g_signal_connect(tab->close_button, "clicked", G_CALLBACK(on_close_tab_clicked), app);
  g_signal_connect(tab->web_view, "decide-policy", G_CALLBACK(on_decide_policy), app);
  g_signal_connect(tab->web_view, "permission-request", G_CALLBACK(on_permission_request), app);
  g_signal_connect(tab->web_view, "enter-fullscreen", G_CALLBACK(on_enter_fullscreen), app);
  g_signal_connect(tab->web_view, "leave-fullscreen", G_CALLBACK(on_leave_fullscreen), app);
  
  // Show the web view
  gtk_widget_show(GTK_WIDGET(tab->web_view));
  
  // Add to notebook
  gint page_num = gtk_notebook_append_page(app->notebook, GTK_WIDGET(tab->web_view), tab->tab_label);
  gtk_notebook_set_current_page(app->notebook, page_num);
  
  // Add to tabs list
  app->tabs = g_list_append(app->tabs, tab);
  app->current_tab = tab;
  
  // Update URL bar for the new tab
  update_url_bar(app, tab);
  
  // Load URI if provided
  if (uri && strlen(uri) > 0) {
    char full_uri[2048];
    if (!strchr(uri, ':')) {
      snprintf(full_uri, sizeof(full_uri), "https://%s", uri);
    } else {
      strncpy(full_uri, uri, sizeof(full_uri) - 1);
      full_uri[sizeof(full_uri) - 1] = '\0';
    }
    webkit_web_view_load_uri(tab->web_view, full_uri);
  }
  
  return tab;
}

// Close tab callback
static void on_close_tab_clicked(GtkButton *button, BrowserApp *app) {
  BrowserTab *tab = (BrowserTab *)g_object_get_data(G_OBJECT(button), "tab-data");
  if (!tab) {
    // Try to find tab by iterating
    GList *iter;
    for (iter = app->tabs; iter != NULL; iter = iter->next) {
      BrowserTab *t = (BrowserTab *)iter->data;
      if (t->close_button == GTK_WIDGET(button)) {
        tab = t;
        break;
      }
    }
  }
  if (tab) {
    close_tab(app, tab);
  }
}

// Close tab
static void close_tab(BrowserApp *app, BrowserTab *tab) {
  if (!tab) return;
  
  // Find page number
  gint page_num = gtk_notebook_page_num(app->notebook, GTK_WIDGET(tab->web_view));
  if (page_num >= 0) {
    gtk_notebook_remove_page(app->notebook, page_num);
  }
  
  // Remove from list
  app->tabs = g_list_remove(app->tabs, tab);
  
  // If this was current tab, switch to another
  if (app->current_tab == tab) {
    if (app->tabs) {
      app->current_tab = (BrowserTab *)app->tabs->data;
      gint new_page = gtk_notebook_page_num(app->notebook, GTK_WIDGET(app->current_tab->web_view));
      if (new_page >= 0) {
        gtk_notebook_set_current_page(app->notebook, new_page);
      }
    } else {
      app->current_tab = NULL;
      // Create new tab if no tabs left
      create_new_tab(app, DEFAULT_HOME);
    }
  }
  
  // Free tab resources
  g_free(tab->title);
  g_free(tab->uri);
  g_free(tab);
  
  // Update UI
  if (app->current_tab) {
    update_url_bar(app, app->current_tab);
  }
}

// Switch to tab
static void switch_to_tab(BrowserApp *app, BrowserTab *tab) {
  if (!tab) return;
  app->current_tab = tab;
  gint page_num = gtk_notebook_page_num(app->notebook, GTK_WIDGET(tab->web_view));
  if (page_num >= 0) {
    gtk_notebook_set_current_page(app->notebook, page_num);
    update_url_bar(app, tab);
  }
}

// Update URL bar
static void update_url_bar(BrowserApp *app, BrowserTab *tab) {
  if (!tab || !app->url_entry || !tab->web_view) return;
  
  const gchar *uri = webkit_web_view_get_uri(tab->web_view);
  if (uri && strlen(uri) > 0) {
    gtk_entry_set_text(app->url_entry, uri);
    if (tab->uri) {
      g_free(tab->uri);
    }
    tab->uri = g_strdup(uri);
  } else if (tab->uri) {
    // Use stored URI if web_view doesn't have one yet
    gtk_entry_set_text(app->url_entry, tab->uri);
  }
  
  // Update navigation buttons
  gboolean can_go_back = webkit_web_view_can_go_back(tab->web_view);
  gboolean can_go_forward = webkit_web_view_can_go_forward(tab->web_view);
  gtk_widget_set_sensitive(GTK_WIDGET(app->back_button), can_go_back);
  gtk_widget_set_sensitive(GTK_WIDGET(app->forward_button), can_go_forward);
}

// Tab switch handler
static void on_tab_switched(GtkNotebook *notebook, GtkWidget *page, guint page_num, BrowserApp *app) {
  GList *iter;
  gint current_page = gtk_notebook_get_current_page(notebook);
  
  for (iter = app->tabs; iter != NULL; iter = iter->next) {
    BrowserTab *tab = (BrowserTab *)iter->data;
    gint tab_page = gtk_notebook_page_num(notebook, GTK_WIDGET(tab->web_view));
    if (tab_page == current_page) {
      switch_to_tab(app, tab);
      break;
    }
  }
}

// URI changed handler
static void on_uri_changed(WebKitWebView *web_view, GParamSpec *spec, BrowserApp *app) {
  BrowserTab *tab = (BrowserTab *)g_object_get_data(G_OBJECT(web_view), "tab-data");
  if (!tab) {
    // Find tab by web_view
    GList *iter;
    for (iter = app->tabs; iter != NULL; iter = iter->next) {
      BrowserTab *t = (BrowserTab *)iter->data;
      if (t->web_view == web_view) {
        tab = t;
        g_object_set_data(G_OBJECT(web_view), "tab-data", tab);
        break;
      }
    }
  }
  
  if (tab && app->current_tab == tab) {
    update_url_bar(app, tab);
    // History will be added when page finishes loading (in on_load_changed)
  }
}

// Title changed handler
static void on_title_changed(WebKitWebView *web_view, GParamSpec *spec, BrowserApp *app) {
  BrowserTab *tab = NULL;
  GList *iter;
  
  for (iter = app->tabs; iter != NULL; iter = iter->next) {
    BrowserTab *t = (BrowserTab *)iter->data;
    if (t->web_view == web_view) {
      tab = t;
      break;
    }
  }
  
  if (!tab) return;
  
  const gchar *title = webkit_web_view_get_title(web_view);
  if (title) {
    g_free(tab->title);
    tab->title = g_strdup(title);
    
    // Update tab label
    GtkBox *label_box = GTK_BOX(tab->tab_label);
    GList *children = gtk_container_get_children(GTK_CONTAINER(label_box));
    if (children) {
      GtkLabel *label = GTK_LABEL(children->data);
      gchar *short_title = g_strdup(title);
      if (strlen(short_title) > 20) {
        short_title[17] = '.';
        short_title[18] = '.';
        short_title[19] = '.';
        short_title[20] = '\0';
      }
      gtk_label_set_text(label, short_title);
      g_free(short_title);
      g_list_free(children);
    }
    
    // Update window title if this is current tab
    if (app->current_tab == tab) {
      gchar window_title[256];
      snprintf(window_title, sizeof(window_title), "%s - %s", title, BROWSER_NAME);
    gtk_window_set_title(app->main_window, window_title);
    }
  }
}

// Load changed handler
static void on_load_changed(WebKitWebView *web_view, WebKitLoadEvent load_event, BrowserApp *app) {
  BrowserTab *tab = NULL;
  GList *iter;
  
  for (iter = app->tabs; iter != NULL; iter = iter->next) {
    BrowserTab *t = (BrowserTab *)iter->data;
    if (t->web_view == web_view) {
      tab = t;
      break;
    }
  }
  
  if (!tab || app->current_tab != tab) return;
  
  // Update navigation buttons
  gboolean can_go_back = webkit_web_view_can_go_back(web_view);
  gboolean can_go_forward = webkit_web_view_can_go_forward(web_view);
  gtk_widget_set_sensitive(GTK_WIDGET(app->back_button), can_go_back);
  gtk_widget_set_sensitive(GTK_WIDGET(app->forward_button), can_go_forward);
  
  // Add to history when page finishes loading
  if (load_event == WEBKIT_LOAD_FINISHED) {
    const gchar *uri = webkit_web_view_get_uri(web_view);
    const gchar *title = webkit_web_view_get_title(web_view);
    if (uri && strlen(uri) > 0 && strncmp(uri, "about:", 6) != 0) {
      add_to_history(app, uri, title);
    }
  }
}

// Navigation callbacks
static void on_back_clicked(GtkButton *button, BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    webkit_web_view_go_back(app->current_tab->web_view);
  }
}

static void on_forward_clicked(GtkButton *button, BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    webkit_web_view_go_forward(app->current_tab->web_view);
  }
}

static void on_refresh_clicked(GtkButton *button, BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    webkit_web_view_reload(app->current_tab->web_view);
  }
}

static void on_home_clicked(GtkButton *button, BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    webkit_web_view_load_uri(app->current_tab->web_view, DEFAULT_HOME);
  }
}

static void activate_uri(BrowserApp *app) {
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

static void on_entry_activated(GtkEntry *entry, BrowserApp *app) {
  activate_uri(app);
}

// New tab callback
static void on_new_tab(GtkMenuItem *item, BrowserApp *app) {
  create_new_tab(app, DEFAULT_HOME);
}

// Close tab menu callback
static void on_close_tab_menu(GtkMenuItem *item, BrowserApp *app) {
  if (app->current_tab) {
    close_tab(app, app->current_tab);
  }
}

// History window
static void show_history_window(GtkMenuItem *item, BrowserApp *app) {
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

static void on_history_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, BrowserApp *app) {
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

// Bookmarks window
static void show_bookmarks_window(GtkMenuItem *item, BrowserApp *app) {
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

static void on_bookmark_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, BrowserApp *app) {
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

// Add bookmark callback
static void on_add_bookmark(GtkMenuItem *item, BrowserApp *app) {
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

// Download handler
static gboolean on_decide_policy(WebKitWebView *web_view, WebKitPolicyDecision *decision,
                                 WebKitPolicyDecisionType decision_type, BrowserApp *app) {
  if (decision_type == WEBKIT_POLICY_DECISION_TYPE_RESPONSE) {
    WebKitResponsePolicyDecision *response_decision = WEBKIT_RESPONSE_POLICY_DECISION(decision);
    WebKitURIResponse *response = webkit_response_policy_decision_get_response(response_decision);
    
    // Check if this is a download
    const gchar *content_type = webkit_uri_response_get_mime_type(response);
    if (content_type && (g_str_has_prefix(content_type, "application/") ||
                         g_str_has_prefix(content_type, "video/") ||
                         g_str_has_prefix(content_type, "audio/"))) {
      // Handle download - use policy decision to download
      webkit_policy_decision_download(decision);
      return TRUE;
    }
  }
  return FALSE;
}

// Permission request handler
static gboolean on_permission_request(WebKitWebView *web_view, WebKitPermissionRequest *request, BrowserApp *app) {
  if (WEBKIT_IS_USER_MEDIA_PERMISSION_REQUEST(request)) {
    webkit_permission_request_allow(request);
    return TRUE;
  }
  return FALSE;
}

// Fullscreen handlers
static gboolean on_enter_fullscreen(WebKitWebView *web_view, BrowserApp *app) {
  return FALSE;
}

static gboolean on_leave_fullscreen(WebKitWebView *web_view, BrowserApp *app) {
  return FALSE;
}

// Download started handler
static void on_download_started(WebKitWebContext *context, WebKitDownload *download, BrowserApp *app) {
  const char *home = g_get_home_dir();
  char download_dir[2048];
  snprintf(download_dir, sizeof(download_dir), "%s/Downloads", home);
  g_mkdir_with_parents(download_dir, 0755);
  
  WebKitURIRequest *request = webkit_download_get_request(download);
  const gchar *uri = webkit_uri_request_get_uri(request);
  
  // Generate filename from URI
  char filepath[2048];
  if (uri) {
    const gchar *basename = strrchr(uri, '/');
    if (basename && strlen(basename) > 1) {
      basename++;
      // Remove query parameters
      const gchar *query = strchr(basename, '?');
      if (query) {
        size_t len = query - basename;
        char filename[512];
        strncpy(filename, basename, len);
        filename[len] = '\0';
        snprintf(filepath, sizeof(filepath), "%s/%s", download_dir, filename);
      } else {
        snprintf(filepath, sizeof(filepath), "%s/%s", download_dir, basename);
      }
    } else {
      snprintf(filepath, sizeof(filepath), "%s/download", download_dir);
    }
  } else {
    snprintf(filepath, sizeof(filepath), "%s/download", download_dir);
  }
  
  webkit_download_set_destination(download, filepath);
}

static gboolean force_redraw(BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    gtk_widget_queue_draw(GTK_WIDGET(app->current_tab->web_view));
  }
  return FALSE;
}

static gboolean on_window_state_changed(GtkWidget *widget, GdkEventWindowState *event, BrowserApp *app) {
  g_idle_add((GSourceFunc)force_redraw, app);
  return FALSE;
}

// Zoom controls
static void on_zoom_in(GtkMenuItem *item, BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    app->zoom_level += 0.1;
    if (app->zoom_level > 3.0) app->zoom_level = 3.0;
    webkit_web_view_set_zoom_level(app->current_tab->web_view, app->zoom_level);
  }
}

static void on_zoom_out(GtkMenuItem *item, BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    app->zoom_level -= 0.1;
    if (app->zoom_level < 0.25) app->zoom_level = 0.25;
    webkit_web_view_set_zoom_level(app->current_tab->web_view, app->zoom_level);
  }
}

static void on_zoom_reset(GtkMenuItem *item, BrowserApp *app) {
  if (app->current_tab && app->current_tab->web_view) {
    app->zoom_level = 1.0;
    webkit_web_view_set_zoom_level(app->current_tab->web_view, 1.0);
  }
}

// Setup persistent storage
static void setup_persistent_storage(WebKitWebContext *context) {
  const char *home = g_get_home_dir();
  char data_dir[2048];
  char cache_dir[2048];
  
  snprintf(data_dir, sizeof(data_dir), "%s/.local/share/vaxp-browser", home);
  snprintf(cache_dir, sizeof(cache_dir), "%s/.cache/vaxp-browser", home);
  
  g_mkdir_with_parents(data_dir, 0700);
  g_mkdir_with_parents(cache_dir, 0700);
  
  WebKitCookieManager *cookie_manager = webkit_web_context_get_cookie_manager(context);
  char cookies_file[2048];
  snprintf(cookies_file, sizeof(cookies_file), "%s/cookies.sqlite", data_dir);
  webkit_cookie_manager_set_persistent_storage(cookie_manager, cookies_file,
                                               WEBKIT_COOKIE_PERSISTENT_STORAGE_SQLITE);
  webkit_cookie_manager_set_accept_policy(cookie_manager, WEBKIT_COOKIE_POLICY_ACCEPT_ALWAYS);
  
  char web_storage_dir[2048];
  snprintf(web_storage_dir, sizeof(web_storage_dir), "%s/storage", data_dir);
  g_mkdir_with_parents(web_storage_dir, 0700);
  // Disable sandbox for better compatibility with websites
  webkit_web_context_set_sandbox_enabled(context, FALSE);
  webkit_web_context_set_cache_model(context, WEBKIT_CACHE_MODEL_WEB_BROWSER);
}

// Create menu bar
static GtkMenuBar* create_menu_bar(BrowserApp *app) {
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
  
  return menu_bar;
}

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
