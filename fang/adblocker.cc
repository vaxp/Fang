#include "adblocker.h"
#include "fingerprint_profiles.h"
#include "privacy_script.h"
#include "network_blocker.h"
#include "adblockplus_integration.h"
#include <stdio.h>
#include <string.h>

// Enhanced ad blocking rules based on EasyList, EasyPrivacy, Fanboy Lists
// https://easylist.to/ - Industry standard filter lists
// Minimal Content Blocker JSON - JavaScript-based ad blocking is primary method
// This is a simple fallback; the real ad blocking happens via injected JavaScript
static const char *DEFAULT_ADS_JSON = 
"["
"  {"
"    \"trigger\": {\"url-filter\": \".*doubleclick\\\\.net.*\"},"
"    \"action\": {\"type\": \"block\"}"
"  },"
"  {"
"    \"trigger\": {\"url-filter\": \".*google-analytics\\\\.com.*\"},"
"    \"action\": {\"type\": \"block\"}"
"  },"
"  {"
"    \"trigger\": {\"url-filter\": \".*facebook\\\\.net.*\"},"
"    \"action\": {\"type\": \"block\"}"
"  },"
"  {"
"    \"trigger\": {\"url-filter\": \".*adnxs\\\\.com.*\"},"
"    \"action\": {\"type\": \"block\"}"
"  },"
"  {"
"    \"trigger\": {\"url-filter\": \".*adsrvr\\\\.org.*\"},"
"    \"action\": {\"type\": \"block\"}"
"  },"
"  {"
"    \"trigger\": {\"url-filter\": \".*taboola\\\\.com.*\"},"
"    \"action\": {\"type\": \"block\"}"
"  },"
"  {"
"    \"trigger\": {\"url-filter\": \".*criteo\\\\.com.*\"},"
"    \"action\": {\"type\": \"block\"}"
"  },"
"  {"
"    \"trigger\": {\"url-filter\": \".*hotjar\\\\.com.*\"},"
"    \"action\": {\"type\": \"block\"}"
"  },"
"  {"
"    \"trigger\": {\"url-filter\": \".*googletagmanager\\\\.com.*\"},"
"    \"action\": {\"type\": \"block\"}"
"  },"
"  {"
"    \"trigger\": {\"url-filter\": \".*/ads/.*\"},"
"    \"action\": {\"type\": \"block\"}"
"  },"
"  {"
"    \"trigger\": {\"url-filter\": \".*/tracking/.*\"},"
"    \"action\": {\"type\": \"block\"}"
"  },"
"  {"
"    \"trigger\": {\"url-filter\": \".*/analytics/.*\"},"
"    \"action\": {\"type\": \"block\"}"
"  }"
"]";

static void on_filter_loaded(WebKitUserContentFilterStore *store, GAsyncResult *result, BrowserApp *app);

static void on_filter_saved(WebKitUserContentFilterStore *store, GAsyncResult *result, BrowserApp *app) {
  GError *error = NULL;
  WebKitUserContentFilter *filter = webkit_user_content_filter_store_save_finish(store, result, &error);
  if (filter) {
    g_print("AdBlocker: Rules compiled and saved successfully: %s\n", webkit_user_content_filter_get_identifier(filter));
    
    // Immediately apply the compiled filter
    GList *iter;
    for (iter = app->tabs; iter != NULL; iter = iter->next) {
      BrowserTab *tab = (BrowserTab *)iter->data;
      WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(tab->web_view);
      if (app->adblock_enabled) {
        webkit_user_content_manager_add_filter(manager, filter);
      }
    }
    
    g_object_unref(filter);
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
  
  // Initialize AdblockPlus integration with powerful EasyList + EasyPrivacy rules
  adblockplus_init();
  
  app->filter_store = webkit_user_content_filter_store_new(data_dir);
  
  // Load all filters
  const char *filters[] = {"ads", "privacy", "annoyance", "unbreak"};
  for (int i = 0; i < 4; i++) {
      char filename[256];
      snprintf(filename, sizeof(filename), "fang/blocked_content_%s.json", filters[i]);
      
      char *json_content = NULL;
      gsize json_len = 0;
      GError *file_error = NULL;
      
      // Check if we need to compile (simplification: always try to load first, if fail then compile? 
      // Or just compile if file exists? Compiling is expensive. 
      // Better: Try to load. If load fails, then compile.
      // But here we are in init.
      
      // Strategy:
      // 1. Trigger a load for each filter.
      // 2. If load fails (in callback), trigger a compile from JSON.
      // BUT, we want to update if the JSON changed.
      // For now, let's just ALWAYS compile if the JSON file exists, because we just generated it.
      // This ensures we are using the latest rules.
      
      if (g_file_get_contents(filename, &json_content, &json_len, &file_error)) {
          g_print("AdBlocker: Loading %lu bytes of rules from %s...\n", json_len, filename);
          GBytes *json_bytes = g_bytes_new_take(json_content, json_len);
          webkit_user_content_filter_store_save(app->filter_store, filters[i], json_bytes, NULL, 
                                                (GAsyncReadyCallback)on_filter_saved, app);
          g_bytes_unref(json_bytes);
      } else {
          // JSON file not found, try to load existing compiled filter from store
          g_warning("AdBlocker: JSON %s not found. Trying to load existing filter...", filename);
          g_error_free(file_error);
          webkit_user_content_filter_store_load(app->filter_store, filters[i], NULL,
                                                (GAsyncReadyCallback)on_filter_loaded, app);
      }
  }

  app->adblock_enabled = TRUE; // Enable by default
  app->privacy_enabled = TRUE; // Enable by default
  
  // Initialize network-level blocker with AdblockPlus rules
  network_blocker_init(app);
  
  // Log AdblockPlus statistics
  g_print("AdBlocker: Total blocking rules: %u\n", adblockplus_get_rule_count());
  g_print("AdBlocker: CSS hiding selectors: %u\n", adblockplus_get_selector_count());
  g_print("AdBlocker: Initializing filter store...\n");
  
  // We don't call adblocker_enable here because we triggered the saves/loads above.
  // The callbacks will handle the application.
}

static void on_filter_loaded(WebKitUserContentFilterStore *store, GAsyncResult *result, BrowserApp *app) {
  GError *error = NULL;
  WebKitUserContentFilter *filter = webkit_user_content_filter_store_load_finish(store, result, &error);
  if (filter) {
    g_print("AdBlocker: Filter loaded: %s\n", webkit_user_content_filter_get_identifier(filter));
    
    // Add to our list of active filters
    app->active_filters = g_list_append(app->active_filters, filter);
    // Note: We keep the reference in the list, so we DON'T unref it at the end of this function
    // unless we remove it from the list.
    
    // Apply to all existing tabs
    GList *iter;
    for (iter = app->tabs; iter != NULL; iter = iter->next) {
      BrowserTab *tab = (BrowserTab *)iter->data;
      WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(tab->web_view);
      
      if (app->adblock_enabled) {
        webkit_user_content_manager_add_filter(manager, filter);
      }
    }
    
    // No unref here because we stored it in app->active_filters
    
  } else {
    // If not found, it might be compiling, or failed.
    if (app->adblock_enabled) {
        g_warning("AdBlocker: Failed to load filter: %s", error->message);
    }
    g_error_free(error);
  }
}

void adblocker_enable(BrowserApp *app, gboolean enable) {
  app->adblock_enabled = enable;
  
  if (enable) {
      // Clear existing list to avoid duplicates if re-enabling?
      // Or assume list is empty if disabled?
      if (app->active_filters) {
          g_list_free_full(app->active_filters, g_object_unref);
          app->active_filters = NULL;
      }

      // Load all filters
      const char *filters[] = {"ads", "privacy", "annoyance", "unbreak"};
      for (int i = 0; i < 4; i++) {
          webkit_user_content_filter_store_load(app->filter_store, filters[i], NULL,
                                                (GAsyncReadyCallback)on_filter_loaded, app);
      }
  } else {
      // Remove all filters from all tabs
      GList *iter;
      for (iter = app->tabs; iter != NULL; iter = iter->next) {
        BrowserTab *tab = (BrowserTab *)iter->data;
        WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(tab->web_view);
        webkit_user_content_manager_remove_all_filters(manager);
      }
      
      // Clear active filters list
      if (app->active_filters) {
          g_list_free_full(app->active_filters, g_object_unref);
          app->active_filters = NULL;
      }
  }
}

gboolean adblocker_reload_filter(BrowserApp *app) {
  // This might not be needed if we handle init correctly, but keeping it for safety
  // Only reload if we have no filters loaded yet?
  if (app->adblock_enabled && !app->active_filters) { // Check if list is empty
      g_print("AdBlocker: Reloading filters (retry)...\n");
      adblocker_enable(app, TRUE);
  }
  return FALSE; 
}

void privacy_enable(BrowserApp *app, gboolean enable) {
  app->privacy_enabled = enable;
  
  // Iterate over all tabs and apply settings
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
  
  // Remove existing privacy scripts
  webkit_user_content_manager_remove_all_scripts(manager);
  
  if (app->privacy_enabled && app->current_profile) {
    // Set User-Agent from profile
    webkit_settings_set_user_agent(settings, app->current_profile->user_agent);
      
    // Generate and inject comprehensive privacy script
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
    
    // Generate and inject ad/tracker blocking script
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
    // Reset to default
    webkit_settings_set_user_agent(settings, NULL);
  }
}

// ========== Fingerprint Management Functions ==========

static gboolean on_profile_rotation_timer(gpointer user_data) {
  BrowserApp *app = (BrowserApp *)user_data;
  fingerprint_rotate_profile(app);
  return TRUE; // Continue timer
}

void fingerprint_init(BrowserApp *app) {
  // Initialize profile pool
  fingerprint_profiles_init();
  
  // Select initial random profile
  app->current_profile = fingerprint_get_random_profile();
  
  // Set rotation interval (5 seconds for aggressive, 0 for per-session only)
  app->rotation_interval_seconds = 5; // Default to 5-second rotation as requested
  app->webrtc_leak_protection = TRUE;
  app->blocked_requests_count = 0;
  app->session_start_time = time(NULL);
  
  // Start rotation timer if interval > 0
  if (app->rotation_interval_seconds > 0) {
    app->profile_rotation_timer_id = g_timeout_add_seconds(
      app->rotation_interval_seconds,
      on_profile_rotation_timer,
      app
    );
    g_print("Fingerprint: Profile rotation enabled (every %d seconds)\n", 
            app->rotation_interval_seconds);
  } else {
    app->profile_rotation_timer_id = 0;
    g_print("Fingerprint: Per-session rotation mode\n");
  }
  
  if (app->current_profile) {
    g_print("Fingerprint: Initial profile: %s\n", app->current_profile->profile_name);
  }
}

void fingerprint_rotate_profile(BrowserApp *app) {
  if (!app) return;
  
  // Select new random profile
  FingerprintProfile *old_profile = app->current_profile;
  app->current_profile = fingerprint_get_random_profile();
  
  // Ensure we got a different profile
  gint attempts = 0;
  while (app->current_profile == old_profile && attempts < 10) {
    app->current_profile = fingerprint_get_random_profile();
    attempts++;
  }
  
  if (app->current_profile) {
    g_print("Fingerprint: Rotated to profile: %s\n", app->current_profile->profile_name);
    
    // Apply new profile to all existing tabs
    GList *iter;
    for (iter = app->tabs; iter != NULL; iter = iter->next) {
      BrowserTab *tab = (BrowserTab *)iter->data;
      if (tab && tab->web_view) {
        fingerprint_apply_to_webview(tab->web_view, app);
      }
    }
    
    // Optional: Clear storage on rotation (can be configured)
    // This is aggressive but maximizes privacy
    WebKitWebsiteDataManager *data_manager = webkit_web_context_get_website_data_manager(app->web_context);
    if (data_manager) {
      webkit_website_data_manager_clear(
        data_manager,
        (WebKitWebsiteDataTypes)(WEBKIT_WEBSITE_DATA_LOCAL_STORAGE | WEBKIT_WEBSITE_DATA_SESSION_STORAGE),
        0, // All time
        NULL, NULL, NULL
      );
    }
  }
}

void fingerprint_apply_to_webview(WebKitWebView *web_view, BrowserApp *app) {
  if (!web_view || !app || !app->current_profile) return;
  
  // Apply privacy settings with current profile
  apply_privacy_settings(web_view, app);
  
  // Note: The privacy script is injected in apply_privacy_settings
  // which uses the current_profile to generate the script
}

void fingerprint_cleanup(BrowserApp *app) {
  if (!app) return;
  
  // Stop rotation timer
  if (app->profile_rotation_timer_id > 0) {
    g_source_remove(app->profile_rotation_timer_id);
    app->profile_rotation_timer_id = 0;
  }
  
  // Cleanup profile pool
  fingerprint_profiles_cleanup();
  
  app->current_profile = NULL;
  
  g_print("Fingerprint: Cleanup complete\n");
}

