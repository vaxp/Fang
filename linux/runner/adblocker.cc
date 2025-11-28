#include "adblocker.h"
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

// JavaScript to inject for anti-fingerprinting
static const char *PRIVACY_SCRIPT = 
"(() => {"
"  /* Spoof Navigator properties */"
"  const spoofedUserAgent = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36';"
"  const spoofedPlatform = 'Win32';"
"  const spoofedHardwareConcurrency = 4;"
"  const spoofedDeviceMemory = 8;"
""
"  Object.defineProperty(navigator, 'userAgent', { get: () => spoofedUserAgent });"
"  Object.defineProperty(navigator, 'platform', { get: () => spoofedPlatform });"
"  Object.defineProperty(navigator, 'hardwareConcurrency', { get: () => spoofedHardwareConcurrency });"
"  Object.defineProperty(navigator, 'deviceMemory', { get: () => spoofedDeviceMemory });"
"  Object.defineProperty(navigator, 'maxTouchPoints', { get: () => 0 });"
""
"  /* Canvas Fingerprinting Protection (Add noise) */"
"  const originalToDataURL = HTMLCanvasElement.prototype.toDataURL;"
"  HTMLCanvasElement.prototype.toDataURL = function(type, encoderOptions) {"
"    const context = this.getContext('2d');"
"    if (context) {"
"      const width = this.width;"
"      const height = this.height;"
"      const imageData = context.getImageData(0, 0, width, height);"
"      for (let i = 0; i < 10; i++) {"
"        const x = Math.floor(Math.random() * width);"
"        const y = Math.floor(Math.random() * height);"
"        const index = (y * width + x) * 4;"
"        imageData.data[index] = imageData.data[index] + 1;"
"      }"
"      context.putImageData(imageData, 0, 0);"
"    }"
"    return originalToDataURL.apply(this, arguments);"
"  };"
""
"  /* WebGL Fingerprinting Protection */"
"  const getParameter = WebGLRenderingContext.prototype.getParameter;"
"  WebGLRenderingContext.prototype.getParameter = function(parameter) {"
"    // Spoof renderer info"
"    if (parameter === 37445) return 'Google Inc. (NVIDIA)';"
"    if (parameter === 37446) return 'ANGLE (NVIDIA, NVIDIA GeForce GTX 1050 Ti Direct3D11 vs_5_0 ps_5_0, D3D11)';"
"    return getParameter.apply(this, arguments);"
"  };"
"})();";

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
  
  // Remove existing privacy scripts (if any) - simplified by removing all scripts for now
  // In a real app we'd track script objects to remove specific ones
  webkit_user_content_manager_remove_all_scripts(manager);
  
  if (app->privacy_enabled) {
    // Spoof User-Agent
    webkit_settings_set_user_agent(settings, 
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
      
    // Inject Privacy Script
    WebKitUserScript *script = webkit_user_script_new(
      PRIVACY_SCRIPT,
      WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
      WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
      NULL, NULL
    );
    webkit_user_content_manager_add_script(manager, script);
    webkit_user_script_unref(script);
    
  } else {
    // Reset to default
    webkit_settings_set_user_agent(settings, NULL);
  }
}
