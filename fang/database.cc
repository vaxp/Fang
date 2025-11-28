#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initialize_databases(BrowserApp *app) {
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

void add_to_history(BrowserApp *app, const gchar *url, const gchar *title) {
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

void setup_persistent_storage(WebKitWebContext *context) {
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
