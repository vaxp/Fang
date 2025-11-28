#ifndef TYPES_H
#define TYPES_H

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
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
  WebKitUserContentFilterStore *filter_store;
  WebKitUserContentFilter *current_filter;
  gboolean adblock_enabled;
  gboolean privacy_enabled;
} BrowserApp;

#endif // TYPES_H
