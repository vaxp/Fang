#include "adblockplus_integration.h"
#include <stdio.h>
#include <string.h>
#include <glib.h>

// Embedded EasyList + EasyPrivacy rules from adblockpluscore
// These are the most powerful blocking rules from Adblock Plus
static const char *EASYLIST_RULES[] = {
  // ========== EASYLIST CORE RULES ==========
  // Main ad patterns
  "-ad-banner",
  "-advertisement",
  "-ads.",
  "-ads/",
  "-ads?",
  "-ads_",
  "ad-placement",
  "ad-slot",
  "ad-unit",
  "ad-wrapper",
  "/adx/",
  "/banner/",
  "/commercial/",
  "/advertisement/",
  "/promotions/",
  "/sponsored/",
  
  // ========== EASYLIST AD NETWORKS ==========
  "adn.js",
  "adnexus.com",
  "ads.yahoo.com",
  "ads.google.com",
  "ads.facebook.com",
  "ads.twitter.com",
  "ads.linkedin.com",
  "doubleclick.net",
  "googlesyndication.com",
  "googleadservices.com",
  "googletagmanager.com",
  "amazon-adsystem.com",
  "criteo.com",
  "pubmatic.com",
  "openx.net",
  "rubiconproject.com",
  "adnxs.com",
  "adsrvr.org",
  
  // ========== EASYLIST CONTENT DELIVERY ==========
  "taboola.com",
  "outbrain.com",
  "disqus.com/recommendations",
  "zemanta.com",
  "revcontent.com",
  "mgid.com",
  
  // ========== EASYPRIVACY RULES ==========
  // Google Analytics and trackers
  "analytics.google.com",
  "google-analytics.com",
  "googletagservices.com",
  "imasdk.googleapis.com",
  
  // Facebook and Meta tracking
  "facebook.net",
  "facebook.com/tr",
  "connect.facebook.net",
  "fbcdn.net",
  "instagram.com/tr",
  
  // Analytics services
  "mixpanel.com",
  "amplitude.com",
  "segment.com",
  "segment.io",
  "optimizely.com",
  "vwo.com",
  "analytics.twitter.com",
  "ads-twitter.com",
  
  // Session recording and heatmaps
  "hotjar.com",
  "mouseflow.com",
  "inspectlet.com",
  "clicktale.net",
  "sessioncam.com",
  "crazyegg.com",
  "fullstory.com",
  
  // Email tracking
  "open.track.com",
  "track.email",
  "readnotify.com",
  "bananatag.com",
  
  // Data collection and profiling
  "scorecardresearch.com",
  "quantserve.com",
  "bluekai.com",
  "crwdcntrl.net",
  "mathtag.com",
  "rlcdn.com",
  "semasio.net",
  "exelator.com",
  "krxd.net",
  "parsely.com",
  "chartbeat.com",
  
  // Mobile tracking
  "appsflyer.com",
  "branch.io",
  "adjust.com",
  "flurry.com",
  "localytics.com",
  "amplitude.com",
  "braze.com",
  
  // Fingerprinting services
  "fingerprintjs.com",
  "iovation.com",
  "threatmetrix.com",
  "siftscience.com",
  
  // ========== FANBOY'S ANNOYANCE LIST ==========
  // Social media widgets and sharing
  "addthis.com",
  "sharethis.com",
  "facebook.com/plugins",
  "platform.twitter.com",
  "youtube.com/embed",
  "disqus.com",
  "widgets.outbrain.com",
  
  // Cookie notices and consent
  "cookieconsent.com",
  "onetrust.com",
  "trustarc.com",
  
  // Pop-ups and notifications
  "popads.net",
  "popcash.net",
  "propellerads.com",
  "onesignal.com",
  "pushwoosh.com",
  
  // ========== ADDITIONAL TRACKING PATTERNS ==========
  "tracking.co",
  "track.co",
  "telemetry.",
  "/pixel",
  "/beacon",
  "/collect",
  "/log?",
  "/track?",
  "/event?",
  "/ping?",
  "/impression",
  "/pageview",
  "/click",
  
  NULL  // Sentinel
};

// CSS-based ad hiding selectors
static const char *AD_HIDING_SELECTORS[] = {
  ".ad-banner",
  ".ad-box",
  ".ad-container",
  ".ad-frame",
  ".ad-header",
  ".ad-placement",
  ".ad-region",
  ".ad-sidebar",
  ".ad-slot",
  ".ad-space",
  ".ad-unit",
  ".ad-wrapper",
  ".ads",
  ".ads-box",
  ".ads-container",
  ".advertisement",
  ".advertisement-container",
  ".advertising",
  ".banner",
  ".banner-ad",
  ".banner-top",
  ".banner-bottom",
  ".banner-sidebar",
  ".content-ad",
  ".div-gpt-ad",
  ".google-ads",
  ".gpt-ad",
  ".google_ads",
  ".sponsored",
  ".sponsored-link",
  ".promotion",
  ".promotional",
  ".promo-banner",
  ".promo-box",
  "[id*='ad-']",
  "[id*='ads-']",
  "[id*='banner-']",
  "[class*='advertisement']",
  "[data-ad-slot]",
  "[data-ad-format]",
  "iframe[src*='ads']",
  "iframe[src*='doubleclick']",
  "iframe[src*='google']",
  "iframe[src*='facebook']",
  
  NULL  // Sentinel
};

// Initialize adblock plus rules
void adblockplus_init() {
  g_print("AdBlockPlus Integration: Initializing with %d core rules\n", 
          g_strv_length((gchar**)EASYLIST_RULES));
  g_print("AdBlockPlus Integration: CSS hiding rules enabled (%d selectors)\n",
          g_strv_length((gchar**)AD_HIDING_SELECTORS));
}

// Check if URL matches any EasyList rules
gboolean adblockplus_should_block_url(const char *url) {
  if (!url) return FALSE;
  
  // Check against all EASYLIST rules
  for (int i = 0; EASYLIST_RULES[i] != NULL; i++) {
    if (g_strstr_len(url, -1, EASYLIST_RULES[i]) != NULL) {
      return TRUE;
    }
  }
  
  return FALSE;
}

// Get CSS selectors for ad hiding
const char** adblockplus_get_ad_hiding_selectors() {
  return (const char**)AD_HIDING_SELECTORS;
}

// Count blocking rules
guint adblockplus_get_rule_count() {
  guint count = 0;
  for (int i = 0; EASYLIST_RULES[i] != NULL; i++) {
    count++;
  }
  return count;
}

// Count CSS selectors
guint adblockplus_get_selector_count() {
  guint count = 0;
  for (int i = 0; AD_HIDING_SELECTORS[i] != NULL; i++) {
    count++;
  }
  return count;
}

// Generate comprehensive CSS filter string
gchar* adblockplus_generate_css_filter() {
  GString *css = g_string_new("{");
  
  for (int i = 0; AD_HIDING_SELECTORS[i] != NULL; i++) {
    if (i > 0) {
      g_string_append(css, ", ");
    }
    g_string_append(css, AD_HIDING_SELECTORS[i]);
  }
  
  g_string_append(css, " { display: none !important; visibility: hidden !important; } }");
  
  return g_string_free(css, FALSE);
}
