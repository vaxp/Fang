#include "adblocker.h"
#include "fingerprint_profiles.h"
#include "privacy_script.h"
#include "network_blocker.h"
#include "adblockplus_integration.h"
#include <stdio.h>
#include <string.h>

// Enhanced ad blocking rules based on EasyList, EasyPrivacy, Fanboy Lists
static const char *DEFAULT_ADS_JSON = "[]"; // Fallback empty

static void on_filter_loaded(WebKitUserContentFilterStore *store, GAsyncResult *result, BrowserApp *app);

// === FIX #1: Correct Logic for Saved Filters ===
static void on_filter_saved(WebKitUserContentFilterStore *store, GAsyncResult *result, BrowserApp *app) {
  GError *error = NULL;
  WebKitUserContentFilter *filter = webkit_user_content_filter_store_save_finish(store, result, &error);
  
  if (filter) {
    g_print("AdBlocker: Rules compiled and saved successfully: %s\n", webkit_user_content_filter_get_identifier(filter));
    
    // IMPORTANT FIX: Transfer ownership to the app instead of unreffing immediately
    // This keeps the filter alive even if there are no tabs yet.
    app->active_filters = g_list_append(app->active_filters, filter);
    
    // Apply the new filter to any existing tabs (if any)
    GList *iter;
    for (iter = app->tabs; iter != NULL; iter = iter->next) {
      BrowserTab *tab = (BrowserTab *)iter->data;
      WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(tab->web_view);
      if (app->adblock_enabled) {
        webkit_user_content_manager_add_filter(manager, filter);
      }
    }
    
    // REMOVED: g_object_unref(filter); 
    // We do NOT unref here because we stored it in app->active_filters.
    // It will be freed later when we clear that list.
    
  } else {
    g_warning("AdBlocker: Failed to save rules: %s", error->message);
    g_error_free(error);
  }
}

void adblocker_init(BrowserApp *app) {
  const char *home = g_get_home_dir();
  char data_dir[2048];
  snprintf(data_dir, sizeof(data_dir), "%s/.local/share/vaxp-browser/adblock", home);
  g_mkdir_with_parents(data_dir, 0700);
  
  // Initialize AdblockPlus integration
  adblockplus_init();
  
  app->filter_store = webkit_user_content_filter_store_new(data_dir);
  
  // === FIX #2: Cookie Policy for Invisible Trackers ===
  // This grants the "Blocking invisible trackers: Yes" status on EFF
  if (app->web_context) {
      WebKitCookieManager *cm = webkit_web_context_get_cookie_manager(app->web_context);
      // Strict policy: Block all third-party cookies (The root cause of invisible tracking)
      webkit_cookie_manager_set_accept_policy(cm, WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY);
      
      // Enable Intelligent Tracking Prevention (ITP)
      WebKitWebsiteDataManager *dm = webkit_web_context_get_website_data_manager(app->web_context);
      webkit_website_data_manager_set_itp_enabled(dm, TRUE);
      
      g_print("AdBlocker: Third-party cookies blocked & ITP enabled.\n");
  }

  // Load all filters
  const char *filters[] = {"ads", "privacy", "annoyance", "unbreak"};
  for (int i = 0; i < 4; i++) {
      char filename[256];
      snprintf(filename, sizeof(filename), "fang/blocked_content_%s.json", filters[i]);
      
      char *json_content = NULL;
      gsize json_len = 0;
      GError *file_error = NULL;
      
      // Attempt to load JSON source to compile/update
      if (g_file_get_contents(filename, &json_content, &json_len, &file_error)) {
          g_print("AdBlocker: Compiling %lu bytes of rules from %s...\n", json_len, filename);
          GBytes *json_bytes = g_bytes_new_take(json_content, json_len);
          
          // Save (Compile) the filter
          webkit_user_content_filter_store_save(app->filter_store, filters[i], json_bytes, NULL, 
                                                (GAsyncReadyCallback)on_filter_saved, app);
          g_bytes_unref(json_bytes);
      } else {
          // Fallback: Load existing compiled binary if JSON missing
          g_warning("AdBlocker: JSON %s not found. Loading cached filter...", filename);
          if (file_error) g_error_free(file_error);
          
          webkit_user_content_filter_store_load(app->filter_store, filters[i], NULL,
                                                (GAsyncReadyCallback)on_filter_loaded, app);
      }
  }

  app->adblock_enabled = TRUE;
  app->privacy_enabled = TRUE;
  
  network_blocker_init(app);
  
  g_print("AdBlocker: Initialization started (Async compilation running...)\n");
}

// Handler for loading pre-compiled filters (No changes needed here, just logic consistency)
static void on_filter_loaded(WebKitUserContentFilterStore *store, GAsyncResult *result, BrowserApp *app) {
  GError *error = NULL;
  WebKitUserContentFilter *filter = webkit_user_content_filter_store_load_finish(store, result, &error);
  if (filter) {
    g_print("AdBlocker: Filter loaded from cache: %s\n", webkit_user_content_filter_get_identifier(filter));
    
    // Add to our list of active filters
    app->active_filters = g_list_append(app->active_filters, filter);
    
    // Apply to all existing tabs
    GList *iter;
    for (iter = app->tabs; iter != NULL; iter = iter->next) {
      BrowserTab *tab = (BrowserTab *)iter->data;
      WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(tab->web_view);
      
      if (app->adblock_enabled) {
        webkit_user_content_manager_add_filter(manager, filter);
      }
    }
  } else {
    // It's normal to fail here if we are compiling for the first time
    if (error) g_error_free(error);
  }
}

void adblocker_enable(BrowserApp *app, gboolean enable) {
  app->adblock_enabled = enable;
  
  if (enable) {
      if (app->active_filters) {
          g_list_free_full(app->active_filters, g_object_unref);
          app->active_filters = NULL;
      }

      const char *filters[] = {"ads", "privacy", "annoyance", "unbreak"};
      for (int i = 0; i < 4; i++) {
          webkit_user_content_filter_store_load(app->filter_store, filters[i], NULL,
                                                (GAsyncReadyCallback)on_filter_loaded, app);
      }
  } else {
      GList *iter;
      for (iter = app->tabs; iter != NULL; iter = iter->next) {
        BrowserTab *tab = (BrowserTab *)iter->data;
        WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(tab->web_view);
        webkit_user_content_manager_remove_all_filters(manager);
      }
      
      if (app->active_filters) {
          g_list_free_full(app->active_filters, g_object_unref);
          app->active_filters = NULL;
      }
  }
}

gboolean adblocker_reload_filter(BrowserApp *app) {
  if (app->adblock_enabled && !app->active_filters) {
      g_print("AdBlocker: Reloading filters...\n");
      adblocker_enable(app, TRUE);
  }
  return FALSE; 
}

void privacy_enable(BrowserApp *app, gboolean enable) {
  app->privacy_enabled = enable;
  GList *iter;
  for (iter = app->tabs; iter != NULL; iter = iter->next) {
    BrowserTab *tab = (BrowserTab *)iter->data;
    apply_privacy_settings(tab->web_view, app);
  }
}

void apply_privacy_settings(WebKitWebView *web_view, BrowserApp *app) {
  if (!web_view) return;
  
  WebKitSettings *settings = webkit_web_view_get_settings(web_view);
  WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(web_view);
  
  webkit_user_content_manager_remove_all_scripts(manager);
  
  if (app->privacy_enabled && app->current_profile) {
    webkit_settings_set_user_agent(settings, app->current_profile->user_agent);
      
    gchar *privacy_script = generate_privacy_script(app->current_profile);
    if (privacy_script) {
      WebKitUserScript *script = webkit_user_script_new(
        privacy_script,
        WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
        WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
        NULL, NULL
      );
      webkit_user_content_manager_add_script(manager, script);
      webkit_user_script_unref(script);
      free_privacy_script(privacy_script);
    }
    
    if (app->adblock_enabled) {
      gchar *ad_script = generate_ad_blocking_script();
      if (ad_script) {
        WebKitUserScript *script = webkit_user_script_new(
          ad_script,
          WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
          WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
          NULL, NULL
        );
        webkit_user_content_manager_add_script(manager, script);
        webkit_user_script_unref(script);
        free_ad_blocking_script(ad_script);
      }
    }
  } else {
    webkit_settings_set_user_agent(settings, NULL);
  }
}

// ========== Fingerprint Management Functions ==========

static gboolean on_profile_rotation_timer(gpointer user_data) {
  BrowserApp *app = (BrowserApp *)user_data;
  fingerprint_rotate_profile(app);
  return TRUE;
}

void fingerprint_init(BrowserApp *app) {
  fingerprint_profiles_init();
  app->current_profile = fingerprint_get_random_profile();
  app->rotation_interval_seconds = 5; 
  app->webrtc_leak_protection = TRUE;
  app->blocked_requests_count = 0;
  app->session_start_time = time(NULL);
  
  if (app->rotation_interval_seconds > 0) {
    app->profile_rotation_timer_id = g_timeout_add_seconds(
      app->rotation_interval_seconds,
      on_profile_rotation_timer,
      app
    );
    g_print("Fingerprint: Profile rotation enabled (every %d seconds)\n", app->rotation_interval_seconds);
  }
  
  if (app->current_profile) {
    g_print("Fingerprint: Initial profile: %s\n", app->current_profile->profile_name);
  }
}

void fingerprint_rotate_profile(BrowserApp *app) {
  if (!app) return;
  
  FingerprintProfile *old_profile = app->current_profile;
  app->current_profile = fingerprint_get_random_profile();
  
  gint attempts = 0;
  while (app->current_profile == old_profile && attempts < 10) {
    app->current_profile = fingerprint_get_random_profile();
    attempts++;
  }
  
  if (app->current_profile) {
    g_print("Fingerprint: Rotated to profile: %s\n", app->current_profile->profile_name);
    
    GList *iter;
    for (iter = app->tabs; iter != NULL; iter = iter->next) {
      BrowserTab *tab = (BrowserTab *)iter->data;
      if (tab && tab->web_view) {
        fingerprint_apply_to_webview(tab->web_view, app);
      }
    }
    
    WebKitWebsiteDataManager *data_manager = webkit_web_context_get_website_data_manager(app->web_context);
    if (data_manager) {
      webkit_website_data_manager_clear(
        data_manager,
        (WebKitWebsiteDataTypes)(WEBKIT_WEBSITE_DATA_LOCAL_STORAGE | WEBKIT_WEBSITE_DATA_SESSION_STORAGE),
        0, NULL, NULL, NULL
      );
    }
  }
}

void fingerprint_apply_to_webview(WebKitWebView *web_view, BrowserApp *app) {
  if (!web_view || !app || !app->current_profile) return;
  apply_privacy_settings(web_view, app);
}

void fingerprint_cleanup(BrowserApp *app) {
  if (!app) return;
  if (app->profile_rotation_timer_id > 0) {
    g_source_remove(app->profile_rotation_timer_id);
    app->profile_rotation_timer_id = 0;
  }
  fingerprint_profiles_cleanup();
  app->current_profile = NULL;
  g_print("Fingerprint: Cleanup complete\n");
}