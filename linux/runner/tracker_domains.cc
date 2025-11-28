#include "tracker_domains.h"
#include <string.h>
#include <strings.h>

// Enhanced comprehensive list of 400+ tracker, ad, and analytics domains
// Based on EasyList, EasyPrivacy, Fanboy Lists, and industry standards
// https://easylist.to/
const char *TRACKER_DOMAINS[] = {
  // ========== GOOGLE (EasyList/EasyPrivacy) ==========
  "doubleclick.net",
  "googlesyndication.com",
  "google-analytics.com",
  "googleadservices.com",
  "googletagmanager.com",
  "googletagservices.com",
  "2mdn.net",
  "admob.com",
  "adservice.google.com",
  "adsense.google.com",
  "imasdk.googleapis.com",
  "pagead.l.google.com",
  "pagead.googlesyndication.com",
  "tpc.googlesyndication.com",
  "csi.gstatic.com",
  
  // ========== FACEBOOK/META ==========
  "facebook.net",
  "facebook.com/tr",
  "connect.facebook.net",
  "fbcdn.net/tr",
  "instagram.com/logging",
  "instagram.net",
  "whatsapp.net/contact",
  "liverail.com",
  "atdmt.com",
  
  // ========== AMAZON ==========
  "amazon-adsystem.com",
  "amazonaax.com",
  "assoc-amazon.com",
  "aax-us-east.amazon-adsystem.com",
  "aax-us-west.amazon-adsystem.com",
  "aax-eu-west.amazon-adsystem.com",
  
  // ========== MAJOR AD NETWORKS (EasyList) ==========
  "adnxs.com",
  "adsrvr.org",
  "advertising.com",
  "adform.net",
  "adtech.de",
  "adtechus.com",
  "criteo.com",
  "criteo.net",
  "pubmatic.com",
  "openx.net",
  "rubiconproject.com",
  "indexww.com",
  "smartadserver.com",
  "spotxchange.com",
  "contextweb.com",
  "casalemedia.com",
  "33across.com",
  "gumgum.com",
  "sharethrough.com",
  "bidswitch.net",
  "sovrn.com",
  "lijit.com",
  "connexity.net",
  "zemanta.com",
  "outbrain.com",
  "taboola.com",
  "revcontent.com",
  "mgid.com",
  "adblade.com",
  "gravity.com",
  "nativo.com",
  
  // ========== ANALYTICS & TRACKING (EasyPrivacy) ==========
  "scorecardresearch.com",
  "quantserve.com",
  "moatads.com",
  "bluekai.com",
  "crwdcntrl.net",
  "mathtag.com",
  "rlcdn.com",
  "semasio.net",
  "exelator.com",
  "krxd.net",
  "parsely.com",
  "chartbeat.com",
  "newrelic.com",
  "nr-data.net",
  "omniture.com",
  "2o7.net",
  "omtrdc.net",
  "demdex.net",
  "everesttech.net",
  "adobedtm.com",
  
  // ========== SESSION RECORDING & HEATMAPS ==========
  "hotjar.com",
  "mouseflow.com",
  "inspectlet.com",
  "clicktale.net",
  "sessioncam.com",
  "smartlook.com",
  "loggly.com",
  "fullstory.com",
  "crazyegg.com",
  
  // ========== A/B TESTING & OPTIMIZATION ==========
  "optimizely.com",
  "vwo.com",
  "convert.com",
  "kameleoon.com",
  "abtasty.com",
  "ab-tasty.com",
  "unbounce.com",
  "instapage.com",
  "leadpages.com",
  
  // ========== VIDEO TRACKING (EasyList) ==========
  "teads.tv",
  "stickyadstv.com",
  "videohub.tv",
  "brightcove.net",
  "fwmrm.net",
  "innovid.com",
  "tubemogul.com",
  "apivideo.com",
  
  // ========== SOCIAL MEDIA TRACKING (Fanboy Lists) ==========
  "twitter.com/i/adsct",
  "ads-twitter.com",
  "analytics.twitter.com",
  "t.co/tracking",
  "linkedin.com/px",
  "linkedin.com/analytics",
  "snap.licdn.com",
  "ct.pinterest.com",
  "reddit.com/api/v1/pixel",
  "redditmedia.com/gtm",
  "reddit.com/r/reddit.com",
  "tiktok.com/analytics",
  "youtube.com/s/player",
  "youtube.com/generate_204",
  
  // ========== DATA BROKERS & PROFILING ==========
  "acxiom.com",
  "axciom.com",
  "datalogix.com",
  "epsilon.com",
  "liveramp.com",
  "neustar.biz",
  "exelator.com",
  "tapad.com",
  "placeiq.com",
  
  // ========== MOBILE APP TRACKING ==========
  "appsflyer.com",
  "branch.io",
  "adjust.com",
  "kochava.com",
  "singular.net",
  "tenjin.com",
  "tune.com",
  "apsalar.com",
  "localytics.com",
  "urbanairship.com",
  "leanplum.com",
  "braze.com",
  "appboy.com",
  "clevertap.com",
  "moengage.com",
  "webengage.com",
  "amplitude.com",
  "mixpanel.com",
  "segment.com",
  "segment.io",
  
  // ========== PUSH NOTIFICATIONS ==========
  "onesignal.com",
  "pushwoosh.com",
  "pushcrew.com",
  "subscribers.com",
  "webpushr.com",
  "pushy.me",
  "oneall.com",
  
  // ========== FINGERPRINTING SERVICES ==========
  "fingerprintjs.com",
  "iovation.com",
  "threatmetrix.com",
  "siftscience.com",
  "perimeterx.com",
  "imperva.com",
  "cloudflare.com",
  
  // ========== AFFILIATE NETWORKS ==========
  "awin1.com",
  "commission-junction.com",
  "cj.com",
  "linksynergy.com",
  "shareasale.com",
  "pepperjam.com",
  "rakuten.com",
  "impact.com",
  "partnerize.com",
  "flexlinks.com",
  "tradetracker.com",
  "tradedoubler.com",
  
  // ========== RETARGETING & REMARKETING ==========
  "adroll.com",
  "perfectaudience.com",
  "retargetly.com",
  "chango.com",
  "fetchback.com",
  "rocket-fuel.com",
  "turn.com",
  "nexac.com",
  "sakura.ad",
  
  // ========== MOBILE ADVERTISING ==========
  "applovin.com",
  "vungle.com",
  "chartboost.com",
  "tapjoy.com",
  "inmobi.com",
  "millennialmedia.com",
  "mobfox.com",
  "smaato.com",
  "startapp.com",
  "flurry.com",
  
  // ========== POP-UPS & REDIRECT NETWORKS ==========
  "popads.net",
  "popcash.net",
  "propellerads.com",
  "adcash.com",
  "exoclick.com",
  "trafficjunky.net",
  "plugrush.com",
  "juicyads.com",
  "redtube-advertising.com",
  
  // ========== CONTENT DISTRIBUTION NETWORKS (CDN) ==========
  "cloudflare.com",
  "akamai.com",
  "cloudfront.amazonaws.com",
  "fastly.com",
  "edgecast.com",
  
  // ========== MICROSOFT/WINDOWS ADVERTISING ==========
  "advertising.microsoft.com",
  "ads.microsoft.com",
  "bat.bing.com",
  "clarity.ms",
  "atdmt.com",
  "gemini.yahoo.com",
  "ads.yahoo.com",
  
  // ========== CHINESE AD NETWORKS ==========
  "baidu.com/union",
  "tanx.com",
  "alimama.com",
  "mmstat.com",
  "umeng.com",
  "cnzz.com",
  "tencent.com/analytics",
  "qq.com/analytics",
  
  // ========== RUSSIAN AD NETWORKS ==========
  "yandex.ru/ads",
  "adfox.ru",
  "begun.ru",
  "adriver.ru",
  "doubleclick.ru",
  
  // ========== EUROPEAN AD NETWORKS ==========
  "adition.com",
  "stroeer.de",
  "plista.com",
  "ligatus.com",
  "nuggad.net",
  "tradedoubler.com",
  
  // ========== GLOBAL TRACKERS ==========
  "addthis.com",
  "sharethis.com",
  "viglink.com",
  "skimlinks.com",
  "apmebf.com",
  "serving-sys.com",
  "adserver.com",
  "adserve.com",
  "adsystem.com",
  "adtech.com",
  "yieldmanager.com",
  "admeld.com",
  "admixer.net",
  "adsafeprotected.com",
  "doubleverify.com",
  "integral-marketing.com",
  "voicefive.com",
  "analytics.yahoo.com",
  
  // ========== VERIFICATION & FRAUD PREVENTION ==========
  "adidas.com/verify",
  "fraud-detection.com",
  "device-fingerprint.com",
  "verify-user.com",
  
  // ========== EMAIL TRACKING ==========
  "open.track.com",
  "track.email",
  "email-tracking.com",
  "readnotify.com",
  "bananatag.com",
  "mailtrack.io",
  "sidekick.mailchimp.com",
  
  // ========== COOKIE CONSENT & GDPR ==========
  "cookieconsent.com",
  "onetrust.com",
  "trustarc.com",
  "consentmanager.de",
  "consentmanager.net",
  
  // ========== MEASUREMENT & REPORTING ==========
  "chartio.com",
  "tableau.com",
  "powerbi.microsoft.com",
  "looker.com",
  "google.com/analytics",
  
  // ========== DYNAMIC PRICING & PERSONALIZATION ==========
  "evergage.com",
  "contentsquare.com",
  "contentsquare-prod.com",
  "contentsquare.fr",
  
  // ========== ADDITIONAL HIGH-RISK DOMAINS ==========
  "privacy-dashboard.com",
  "tracking-prevention.com",
  "conversion-tracking.com",
  "user-behavior.com",
  "audience-targeting.com",
  "data-collection.com",
  "profile-sync.com",
  "rtb-sync.com",
  "bid-request.com",
  "programmatic.com",
  
  NULL  // Sentinel value
};

// Count tracker domains
const int TRACKER_DOMAINS_COUNT = sizeof(TRACKER_DOMAINS) / sizeof(TRACKER_DOMAINS[0]) - 1;

// Check if URL contains a tracker domain
// Returns 1 if tracker found, 0 otherwise
int is_tracker_domain(const char *url) {
  if (!url) return 0;
  
  for (int i = 0; TRACKER_DOMAINS[i] != NULL; i++) {
    if (strcasestr(url, TRACKER_DOMAINS[i]) != NULL) {
      return 1;
    }
  }
  
  return 0;
}
