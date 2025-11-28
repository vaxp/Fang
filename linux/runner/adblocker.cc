#include "adblocker.h"
#include "fingerprint_profiles.h"
#include "privacy_script.h"
#include <stdio.h>
#include <string.h>

// Basic ad blocking rules in JSON format
// In a real app, this would be loaded from a file or URL (e.g., EasyList)
// Basic ad blocking rules in JSON format
// Expanded to cover more common trackers and ad patterns
static const char *DEFAULT_ADS_JSON = 
"["
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*\""
"    },"
"    \"action\": {"
"      \"type\": \"css-display-none\","
"      \"selector\": \".ad, .ads, .advert, .banner, .banner-ad, #ad, #ads, #advert, #banner, #banner-ad, [id^='google_ads'], [id^='div-gpt-ad'], [class^='ad-'], [class^='ads-'], .pub_300x250, .pub_300x250m, .pub_728x90, .text-ad, .text-ads, .text_ad, .text_ads, .text-ad-links, .ad-container, .ad-slot, .ad-wrapper, .ad-box, .ad-unit, .ad-zone, .ad-space, .ad-frame, .ad-sidebar, .ad-header, .ad-footer, .ad-popup, .ad-overlay, .ad-interstitial, .ad-video, .ad-image, .ad-link, .ad-text, .ad-label, .ad-badge, .ad-mark, .ad-sign, .ad-symbol, .ad-icon, .ad-logo, .ad-button, .ad-cta, .ad-click, .ad-view, .ad-impression, .ad-tracker, .ad-pixel, .ad-script, .ad-code, .ad-tag, .ad-id, .ad-class, .ad-name, .ad-title, .ad-desc, .ad-description, .ad-info, .ad-content, .ad-body, .ad-html, .ad-css, .ad-js, .ad-json, .ad-xml, .ad-data, .ad-url, .ad-uri, .ad-href, .ad-src, .ad-target, .ad-rel, .ad-alt, .ad-width, .ad-height, .ad-size, .ad-type, .ad-format, .ad-layout, .ad-style, .ad-theme, .ad-color, .ad-font, .ad-bg, .ad-background, .ad-border, .ad-margin, .ad-padding, .ad-position, .ad-display, .ad-visibility, .ad-opacity, .ad-z-index, .ad-overflow, .ad-clip, .ad-transform, .ad-transition, .ad-animation\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*doubleclick.net.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*googlesyndication.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*google-analytics.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*facebook.net.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*facebook.com/tr/.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*adnxs.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*adsrvr.org.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*bluekai.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*crwdcntrl.net.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*mathtag.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*rlcdn.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*rubiconproject.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*scorecardresearch.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*semasio.net.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*taboola.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*twitter.com/u/.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*yahoo.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*amazon-adsystem.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*quantserve.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*moatads.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*outbrain.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*criteo.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*pubmatic.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*openx.net.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*smartadserver.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*spotxchange.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*teads.tv.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*stickyadstv.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*contextweb.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*casalemedia.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*33across.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*gumgum.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*sharethrough.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*bidswitch.net.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*sovrn.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*lijit.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*connexity.net.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*zemanta.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*revcontent.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*mgid.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*adblade.com.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  },"
"  {"
"    \"trigger\": {"
"      \"url-filter\": \".*content.ad.*\""
"    },"
"    \"action\": {"
"      \"type\": \"block\""
"    }"
"  }"
"]";

static void on_filter_saved(WebKitUserContentFilterStore *store, GAsyncResult *result, gpointer user_data) {
  GError *error = NULL;
  WebKitUserContentFilter *filter = webkit_user_content_filter_store_save_finish(store, result, &error);
  if (filter) {
    g_print("AdBlocker: Rules compiled and saved successfully.\n");
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
  
  app->filter_store = webkit_user_content_filter_store_new(data_dir);
  
  // Compile default rules
  GBytes *json_bytes = g_bytes_new_static(DEFAULT_ADS_JSON, strlen(DEFAULT_ADS_JSON));
  webkit_user_content_filter_store_save(app->filter_store, "default-ads", json_bytes, NULL, 
                                        (GAsyncReadyCallback)on_filter_saved, NULL);
  g_bytes_unref(json_bytes);
  
  app->adblock_enabled = TRUE; // Enable by default
  app->privacy_enabled = TRUE; // Enable by default
  
  // Load the filter immediately
  adblocker_enable(app, TRUE);
}

static void on_filter_loaded(WebKitUserContentFilterStore *store, GAsyncResult *result, BrowserApp *app) {
  GError *error = NULL;
  WebKitUserContentFilter *filter = webkit_user_content_filter_store_load_finish(store, result, &error);
  if (filter) {
    if (app->current_filter) {
      g_object_unref(app->current_filter);
    }
    app->current_filter = filter; // Keep reference
    
    // Apply to all existing tabs
    GList *iter;
    for (iter = app->tabs; iter != NULL; iter = iter->next) {
      BrowserTab *tab = (BrowserTab *)iter->data;
      WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager(tab->web_view);
      webkit_user_content_manager_remove_all_filters(manager); // Clear old
      if (app->adblock_enabled) {
        webkit_user_content_manager_add_filter(manager, app->current_filter);
      }
    }
    
    if (app->adblock_enabled) {
      g_print("AdBlocker: Enabled.\n");
    } else {
      g_print("AdBlocker: Disabled.\n");
    }
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
  
  // Load the filter (or reload to ensure we have it)
  webkit_user_content_filter_store_load(app->filter_store, "default-ads", NULL,
                                        (GAsyncReadyCallback)on_filter_loaded, app);
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

