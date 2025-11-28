#include "network_blocker.h"
#include "tracker_domains.h"
#include "adblockplus_integration.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

// Enhanced URL patterns to block (EasyList/EasyPrivacy compatible)
static const char *BLOCKED_URL_PATTERNS[] = {
  // Analytics & Tracking
  "/ads/",
  "/ad/",
  "/advert/",
  "/advertising/",
  "/tracker/",
  "/tracking/",
  "/analytics/",
  "/pixel.",
  "/beacon/",
  "/collect?",
  "/track?",
  "/log?",
  "/event?",
  "/impression?",
  "/telemetry/",
  "/report?",
  "/pageview?",
  "/session?",
  "/conversion?",
  
  // Google tracking
  "googletagmanager",
  "google-analytics",
  "facebook.com/tr",
  "doubleclick",
  "googleadservices",
  "imasdk.googleapis.com",
  
  // Script patterns
  "/gtm.js",
  "/ga.js",
  "/analytics.js",
  "/fbevents.js",
  "/pixel.gif",
  "/tracking.gif",
  "/1x1.gif",
  "/transparent.gif",
  "/beacon.gif",
  "/b.gif",
  "/ct.gif",
  "/count.gif",
  
  // CNAME cloaking detection
  "cdn-js",
  "analytics-js",
  "cdn-analytics",
  "tracking-cdn",
  
  // Common tracking parameters
  "?utm_",
  "&utm_",
  "?fbclid=",
  "&fbclid=",
  "?gclid=",
  "&gclid=",
  "?msclkid=",
  "&msclkid=",
  
  NULL
};

// Enhanced script patterns to block
static const char *BLOCKED_SCRIPT_PATTERNS[] = {
  "ads.js",
  "ad.js",
  "advertising.js",
  "tracker.js",
  "tracking.js",
  "analytics.js",
  "ga.js",
  "gtm.js",
  "fbevents.js",
  "pixel.js",
  "beacon.js",
  "event.js",
  "collect.js",
  "event-logger.js",
  "user-track.js",
  "tracking-pixel.js",
  "analytics-pixel.js",
  "monitor.js",
  "report.js",
  "telemetry.js",
  NULL
};

void network_blocker_init(BrowserApp *app) {
  if (!app) return;
  
  // Initialize blocked requests counter
  app->blocked_requests_count = 0;
  
  g_print("Network Blocker: Initialized with %d tracker domains\n", TRACKER_DOMAINS_COUNT);
  g_print("Network Blocker: Ready to intercept requests\n");
}

gboolean should_block_request(const char *uri) {
  if (!uri) return FALSE;
  
  // Layer 1: Check against tracker domains
  if (is_tracker_domain(uri)) {
    return TRUE;
  }
  
  // Layer 2: Check against URL patterns
  for (int i = 0; BLOCKED_URL_PATTERNS[i] != NULL; i++) {
    if (strcasestr(uri, BLOCKED_URL_PATTERNS[i]) != NULL) {
      return TRUE;
    }
  }
  
  // Layer 3: Check if it's a script with blocked pattern
  if (strstr(uri, ".js")) {
    for (int i = 0; BLOCKED_SCRIPT_PATTERNS[i] != NULL; i++) {
      if (strcasestr(uri, BLOCKED_SCRIPT_PATTERNS[i]) != NULL) {
        return TRUE;
      }
    }
  }
  
  // Layer 4: Check against AdblockPlus EasyList/EasyPrivacy rules
  if (adblockplus_should_block_url(uri)) {
    return TRUE;
  }
  
  return FALSE;
}

gboolean on_send_request(WebKitWebView *web_view, WebKitURIRequest *request,
                         WebKitURIResponse *redirected_response, gpointer user_data) {
  BrowserApp *app = (BrowserApp *)user_data;
  if (!app || !request) return FALSE;
  
  const char *uri = webkit_uri_request_get_uri(request);
  if (!uri) return FALSE;
  
  // Check if request should be blocked
  if (should_block_request(uri)) {
    // Increment blocked counter
    app->blocked_requests_count++;
    
    // Log blocked request (can be disabled for performance)
    if (app->blocked_requests_count % 100 == 0) {
      g_print("Network Blocker: Blocked %lu requests\n", app->blocked_requests_count);
    }
    
    // Return TRUE to block the request
    return TRUE;
  }
  
  // Allow request
  return FALSE;
}

guint64 get_blocked_requests_count(BrowserApp *app) {
  return app ? app->blocked_requests_count : 0;
}
