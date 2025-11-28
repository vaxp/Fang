#include "tabs.h"
#include "database.h"
#include "adblocker.h"
#include <string.h>
#include <stdio.h>

BrowserTab* create_new_tab(BrowserApp *app, const gchar *uri) {
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
  
  // Apply privacy settings
  apply_privacy_settings(tab->web_view, app);
  
  // Apply fingerprint profile
  fingerprint_apply_to_webview(tab->web_view, app);
  
  // Apply AdBlocker if enabled
  if (app->adblock_enabled && app->current_filter) {
    WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(tab->web_view);
    webkit_user_content_manager_add_filter(manager, app->current_filter);
  }
  
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

void close_tab(BrowserApp *app, BrowserTab *tab) {
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

void switch_to_tab(BrowserApp *app, BrowserTab *tab) {
  if (!tab) return;
  app->current_tab = tab;
  gint page_num = gtk_notebook_page_num(app->notebook, GTK_WIDGET(tab->web_view));
  if (page_num >= 0) {
    gtk_notebook_set_current_page(app->notebook, page_num);
    update_url_bar(app, tab);
  }
}

void update_url_bar(BrowserApp *app, BrowserTab *tab) {
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

void on_tab_switched(GtkNotebook *notebook, GtkWidget *page, guint page_num, BrowserApp *app) {
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

void on_uri_changed(WebKitWebView *web_view, GParamSpec *spec, BrowserApp *app) {
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

void on_title_changed(WebKitWebView *web_view, GParamSpec *spec, BrowserApp *app) {
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

void on_load_changed(WebKitWebView *web_view, WebKitLoadEvent load_event, BrowserApp *app) {
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

void on_close_tab_clicked(GtkButton *button, BrowserApp *app) {
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

gboolean on_decide_policy(WebKitWebView *web_view, WebKitPolicyDecision *decision,
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

gboolean on_permission_request(WebKitWebView *web_view, WebKitPermissionRequest *request, BrowserApp *app) {
  if (WEBKIT_IS_USER_MEDIA_PERMISSION_REQUEST(request)) {
    webkit_permission_request_allow(request);
    return TRUE;
  }
  return FALSE;
}

gboolean on_enter_fullscreen(WebKitWebView *web_view, BrowserApp *app) {
  return FALSE;
}

gboolean on_leave_fullscreen(WebKitWebView *web_view, BrowserApp *app) {
  return FALSE;
}

void on_download_started(WebKitWebContext *context, WebKitDownload *download, BrowserApp *app) {
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
