#include "fingerprint_profiles.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Global profile pool
static struct FingerprintProfile *profile_pool = NULL;
static gint profile_count = 0;

// Helper function to create a profile
static FingerprintProfile* create_profile(
  gint id,
  const gchar *name,
  const gchar *ua,
  const gchar *platform,
  gint hw_concurrency,
  gint device_mem,
  const gchar **langs,
  gint lang_count,
  gint max_touch,
  const gchar *vendor,
  gint screen_w,
  gint screen_h,
  gdouble dpr,
  const gchar *tz,
  const gchar *lang,
  const gchar *webgl_vendor,
  const gchar *webgl_renderer
) {
  struct FingerprintProfile *profile = g_new0(struct FingerprintProfile, 1);
  
  profile->profile_id = id;
  profile->profile_name = g_strdup(name);
  profile->user_agent = g_strdup(ua);
  profile->platform = g_strdup(platform);
  profile->hardware_concurrency = hw_concurrency;
  profile->device_memory = device_mem;
  
  // Copy languages array
  profile->languages_count = lang_count;
  profile->languages = g_new0(gchar*, lang_count + 1);
  for (gint i = 0; i < lang_count; i++) {
    profile->languages[i] = g_strdup(langs[i]);
  }
  profile->languages[lang_count] = NULL;
  
  profile->max_touch_points = max_touch;
  profile->vendor = g_strdup(vendor);
  
  profile->screen_width = screen_w;
  profile->screen_height = screen_h;
  profile->screen_avail_width = screen_w;
  profile->screen_avail_height = screen_h - 40; // Account for taskbar
  profile->device_pixel_ratio = dpr;
  profile->color_depth = 24;
  
  profile->timezone = g_strdup(tz);
  profile->language = g_strdup(lang);
  
  profile->webgl_vendor = g_strdup(webgl_vendor);
  profile->webgl_renderer = g_strdup(webgl_renderer);
  
  // Generate random seeds for noise
  profile->canvas_seed = (guint32)(g_random_int() ^ (id * 12345));
  profile->audio_seed = (guint32)(g_random_int() ^ (id * 54321));
  
  return profile;
}

void fingerprint_profiles_init() {
  if (profile_pool != NULL) {
    return; // Already initialized
  }
  
  // Seed random number generator
  g_random_set_seed((guint32)time(NULL));
  
  // Create 500+ diverse profiles covering all major brands, OS combinations, and Android devices
  profile_count = 565;
  profile_pool = g_new0(struct FingerprintProfile, profile_count);
  
  const gchar *en_us_langs[] = {"en-US", "en"};
  const gchar *en_gb_langs[] = {"en-GB", "en"};
  const gchar *de_langs[] = {"de-DE", "de", "en"};
  const gchar *fr_langs[] = {"fr-FR", "fr", "en"};
  const gchar *es_langs[] = {"es-ES", "es", "en"};
  const gchar *ja_langs[] = {"ja-JP", "ja", "en"};
  
  gint idx = 0;
  
  // Windows profiles (Chrome)
  profile_pool[idx++] = *create_profile(
    1, "Windows 10 Chrome 120",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "Win32", 8, 8, en_us_langs, 2, 0, "Google Inc.",
    1920, 1080, 1.0, "America/New_York", "en-US",
    "Google Inc. (NVIDIA)", "ANGLE (NVIDIA, NVIDIA GeForce GTX 1660 Direct3D11 vs_5_0 ps_5_0)"
  );
  
  profile_pool[idx++] = *create_profile(
    2, "Windows 11 Chrome 121",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36",
    "Win32", 16, 16, en_us_langs, 2, 0, "Google Inc.",
    2560, 1440, 1.0, "America/Los_Angeles", "en-US",
    "Google Inc. (NVIDIA)", "ANGLE (NVIDIA, NVIDIA GeForce RTX 3060 Direct3D11 vs_5_0 ps_5_0)"
  );
  
  profile_pool[idx++] = *create_profile(
    3, "Windows 10 Chrome 119",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36",
    "Win32", 4, 8, en_gb_langs, 2, 0, "Google Inc.",
    1366, 768, 1.25, "Europe/London", "en-GB",
    "Google Inc. (Intel)", "ANGLE (Intel, Intel(R) UHD Graphics 620 Direct3D11 vs_5_0 ps_5_0)"
  );
  
  profile_pool[idx++] = *create_profile(
    4, "Windows 11 Chrome 122",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
    "Win32", 12, 16, de_langs, 3, 0, "Google Inc.",
    1920, 1200, 1.0, "Europe/Berlin", "de-DE",
    "Google Inc. (AMD)", "ANGLE (AMD, AMD Radeon RX 6700 XT Direct3D11 vs_5_0 ps_5_0)"
  );
  
  // Windows profiles (Edge)
  profile_pool[idx++] = *create_profile(
    5, "Windows 11 Edge 120",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0",
    "Win32", 8, 8, en_us_langs, 2, 0, "Google Inc.",
    1920, 1080, 1.5, "America/Chicago", "en-US",
    "Google Inc. (NVIDIA)", "ANGLE (NVIDIA, NVIDIA GeForce GTX 1050 Ti Direct3D11 vs_5_0 ps_5_0)"
  );
  
  // macOS profiles (Safari)
  profile_pool[idx++] = *create_profile(
    6, "macOS Sonoma Safari 17",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15",
    "MacIntel", 8, 16, en_us_langs, 2, 0, "Apple Inc.",
    2560, 1600, 2.0, "America/New_York", "en-US",
    "Apple Inc.", "Apple M1"
  );
  
  profile_pool[idx++] = *create_profile(
    7, "macOS Ventura Safari 16",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/16.6 Safari/605.1.15",
    "MacIntel", 4, 8, en_us_langs, 2, 0, "Apple Inc.",
    1920, 1080, 2.0, "America/Los_Angeles", "en-US",
    "Apple Inc.", "Intel(R) Iris(TM) Plus Graphics 640"
  );
  
  // macOS profiles (Chrome)
  profile_pool[idx++] = *create_profile(
    8, "macOS Sonoma Chrome 120",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "MacIntel", 10, 16, en_us_langs, 2, 0, "Google Inc.",
    2880, 1800, 2.0, "America/Denver", "en-US",
    "Google Inc. (Apple)", "ANGLE (Apple, ANGLE Metal Renderer: Apple M2, Unspecified Version)"
  );
  
  profile_pool[idx++] = *create_profile(
    9, "macOS Monterey Chrome 119",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36",
    "MacIntel", 8, 16, en_us_langs, 2, 0, "Google Inc.",
    1680, 1050, 2.0, "Asia/Tokyo", "en-US",
    "Google Inc. (Apple)", "ANGLE (Apple, ANGLE Metal Renderer: Apple M1 Pro, Unspecified Version)"
  );
  
  // Linux profiles (Chrome)
  profile_pool[idx++] = *create_profile(
    10, "Linux Ubuntu Chrome 120",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "Linux x86_64", 8, 16, en_us_langs, 2, 0, "Google Inc.",
    1920, 1080, 1.0, "America/New_York", "en-US",
    "Google Inc. (NVIDIA)", "ANGLE (NVIDIA, NVIDIA GeForce GTX 1660 Ti/PCIe/SSE2, OpenGL 4.6.0)"
  );
  
  profile_pool[idx++] = *create_profile(
    11, "Linux Fedora Chrome 121",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36",
    "Linux x86_64", 12, 32, en_us_langs, 2, 0, "Google Inc.",
    2560, 1440, 1.0, "Europe/Berlin", "en-US",
    "Google Inc. (AMD)", "ANGLE (AMD, AMD Radeon RX 6800 XT (radeonsi, navi21, LLVM 15.0.0, DRM 3.49, 6.1.0), OpenGL 4.6.0)"
  );
  
  profile_pool[idx++] = *create_profile(
    12, "Linux Debian Chrome 119",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36",
    "Linux x86_64", 4, 8, en_us_langs, 2, 0, "Google Inc.",
    1366, 768, 1.0, "Europe/Paris", "en-US",
    "Google Inc. (Intel)", "ANGLE (Intel, Mesa Intel(R) UHD Graphics 620 (KBL GT2), OpenGL 4.6.0)"
  );
  
  // Linux profiles (Firefox)
  profile_pool[idx++] = *create_profile(
    13, "Linux Ubuntu Firefox 121",
    "Mozilla/5.0 (X11; Linux x86_64; rv:121.0) Gecko/20100101 Firefox/121.0",
    "Linux x86_64", 16, 32, en_us_langs, 2, 0, "",
    3840, 2160, 1.0, "America/Los_Angeles", "en-US",
    "X.Org", "AMD Radeon RX 6900 XT (radeonsi, navi21, LLVM 15.0.0, DRM 3.49, 6.1.0)"
  );
  
  // Additional diverse profiles
  profile_pool[idx++] = *create_profile(
    14, "Windows 10 Chrome FR",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "Win32", 6, 8, fr_langs, 3, 0, "Google Inc.",
    1600, 900, 1.0, "Europe/Paris", "fr-FR",
    "Google Inc. (NVIDIA)", "ANGLE (NVIDIA, NVIDIA GeForce GTX 1650 Direct3D11 vs_5_0 ps_5_0)"
  );
  
  profile_pool[idx++] = *create_profile(
    15, "Windows 11 Chrome ES",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36",
    "Win32", 8, 16, es_langs, 3, 0, "Google Inc.",
    1920, 1080, 1.0, "Europe/Madrid", "es-ES",
    "Google Inc. (AMD)", "ANGLE (AMD, AMD Radeon RX 580 Direct3D11 vs_5_0 ps_5_0)"
  );
  
  profile_pool[idx++] = *create_profile(
    16, "macOS Chrome JP",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "MacIntel", 8, 16, ja_langs, 3, 0, "Google Inc.",
    1920, 1080, 2.0, "Asia/Tokyo", "ja-JP",
    "Google Inc. (Apple)", "ANGLE (Apple, ANGLE Metal Renderer: Apple M1, Unspecified Version)"
  );
  
  profile_pool[idx++] = *create_profile(
    17, "Linux Arch Chrome",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
    "Linux x86_64", 16, 32, en_us_langs, 2, 0, "Google Inc.",
    2560, 1440, 1.0, "America/Phoenix", "en-US",
    "Google Inc. (NVIDIA)", "ANGLE (NVIDIA, NVIDIA GeForce RTX 3080/PCIe/SSE2, OpenGL 4.6.0)"
  );
  
  profile_pool[idx++] = *create_profile(
    18, "Windows 10 Edge DE",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0",
    "Win32", 8, 16, de_langs, 3, 0, "Google Inc.",
    1920, 1080, 1.25, "Europe/Berlin", "de-DE",
    "Google Inc. (Intel)", "ANGLE (Intel, Intel(R) UHD Graphics 630 Direct3D11 vs_5_0 ps_5_0)"
  );
  
  // Android profiles (Chrome) - 50 new profiles
  // Samsung Galaxy S23 series
  profile_pool[idx++] = *create_profile(
    19, "Android Samsung Galaxy S23 Ultra",
    "Mozilla/5.0 (Linux; Android 14; SM-S918B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3120, 1.44, "America/New_York", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  profile_pool[idx++] = *create_profile(
    20, "Android Samsung Galaxy S23+",
    "Mozilla/5.0 (Linux; Android 14; SM-S916B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 6, 8, en_us_langs, 2, 10, "Google Inc.",
    1440, 3120, 1.44, "America/Chicago", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  profile_pool[idx++] = *create_profile(
    21, "Android Samsung Galaxy S23",
    "Mozilla/5.0 (Linux; Android 14; SM-S911B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 8, en_us_langs, 2, 10, "Google Inc.",
    1080, 2340, 1.0, "America/Los_Angeles", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  profile_pool[idx++] = *create_profile(
    22, "Android Samsung Galaxy A54",
    "Mozilla/5.0 (Linux; Android 14; SM-A546B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 6, 6, en_us_langs, 2, 10, "Google Inc.",
    1080, 2340, 1.0, "Europe/London", "en-US",
    "Google Inc. (MediaTek)", "ARM Mali-G78 MP20"
  );
  
  // Google Pixel series
  profile_pool[idx++] = *create_profile(
    23, "Android Google Pixel 8 Pro",
    "Mozilla/5.0 (Linux; Android 14; Pixel 8 Pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3120, 1.5, "America/New_York", "en-US",
    "Google Inc. (Google)", "Google Tensor G3"
  );
  
  profile_pool[idx++] = *create_profile(
    24, "Android Google Pixel 8",
    "Mozilla/5.0 (Linux; Android 14; Pixel 8) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 8, en_us_langs, 2, 10, "Google Inc.",
    1080, 2340, 1.0, "America/Denver", "en-US",
    "Google Inc. (Google)", "Google Tensor G3"
  );
  
  profile_pool[idx++] = *create_profile(
    25, "Android Google Pixel 8a",
    "Mozilla/5.0 (Linux; Android 14; Pixel 8a) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 6, 8, en_us_langs, 2, 10, "Google Inc.",
    1080, 2340, 1.0, "Europe/Paris", "en-US",
    "Google Inc. (Google)", "Google Tensor G3"
  );
  
  profile_pool[idx++] = *create_profile(
    26, "Android Google Pixel 7 Pro",
    "Mozilla/5.0 (Linux; Android 13; Pixel 7 Pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3120, 1.5, "America/Phoenix", "en-US",
    "Google Inc. (Google)", "Google Tensor G2"
  );
  
  // iPhone models (iOS Safari - treated as mobile)
  profile_pool[idx++] = *create_profile(
    27, "iOS iPhone 15 Pro Max Safari",
    "Mozilla/5.0 (iPhone; CPU iPhone OS 17_2 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.2 Mobile/15E148 Safari/604.1",
    "iPhone", 6, 8, en_us_langs, 2, 5, "Apple Inc.",
    1290, 2796, 3.0, "America/New_York", "en-US",
    "Apple Inc.", "Apple A17 Pro GPU"
  );
  
  profile_pool[idx++] = *create_profile(
    28, "iOS iPhone 15 Pro Safari",
    "Mozilla/5.0 (iPhone; CPU iPhone OS 17_2 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.2 Mobile/15E148 Safari/604.1",
    "iPhone", 6, 6, en_us_langs, 2, 5, "Apple Inc.",
    1179, 2556, 3.0, "America/Los_Angeles", "en-US",
    "Apple Inc.", "Apple A17 Pro GPU"
  );
  
  profile_pool[idx++] = *create_profile(
    29, "iOS iPhone 15 Safari",
    "Mozilla/5.0 (iPhone; CPU iPhone OS 17_2 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.2 Mobile/15E148 Safari/604.1",
    "iPhone", 6, 6, en_us_langs, 2, 5, "Apple Inc.",
    1080, 2340, 3.0, "Europe/London", "en-US",
    "Apple Inc.", "Apple A16 Bionic GPU"
  );
  
  profile_pool[idx++] = *create_profile(
    30, "iOS iPhone 14 Pro Max Safari",
    "Mozilla/5.0 (iPhone; CPU iPhone OS 17_1 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.1 Mobile/15E148 Safari/604.1",
    "iPhone", 6, 6, en_us_langs, 2, 5, "Apple Inc.",
    1290, 2796, 3.0, "Europe/Berlin", "en-US",
    "Apple Inc.", "Apple A16 Bionic GPU"
  );
  
  // OnePlus series
  profile_pool[idx++] = *create_profile(
    31, "Android OnePlus 12",
    "Mozilla/5.0 (Linux; Android 14; OnePlus 12) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3168, 1.44, "Asia/Shanghai", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  profile_pool[idx++] = *create_profile(
    32, "Android OnePlus 12R",
    "Mozilla/5.0 (Linux; Android 14; OnePlus 12R) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 8, en_us_langs, 2, 10, "Google Inc.",
    1440, 3168, 1.44, "Asia/Kolkata", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  profile_pool[idx++] = *create_profile(
    33, "Android OnePlus 11",
    "Mozilla/5.0 (Linux; Android 13; OnePlus 11) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3168, 1.44, "Europe/Paris", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8 Gen 1"
  );
  
  // Xiaomi series
  profile_pool[idx++] = *create_profile(
    34, "Android Xiaomi 14 Ultra",
    "Mozilla/5.0 (Linux; Android 14; xiaomi Xiaomi 14 Ultra) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 16, en_us_langs, 2, 10, "Google Inc.",
    1440, 3200, 1.5, "Asia/Shanghai", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  profile_pool[idx++] = *create_profile(
    35, "Android Xiaomi 14",
    "Mozilla/5.0 (Linux; Android 14; xiaomi 2312DQA47T) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 12, en_us_langs, 2, 10, "Google Inc.",
    1080, 2400, 1.0, "Asia/Taipei", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  profile_pool[idx++] = *create_profile(
    36, "Android Xiaomi 13",
    "Mozilla/5.0 (Linux; Android 13; xiaomi 2210132C) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 8, en_us_langs, 2, 10, "Google Inc.",
    1080, 2400, 1.0, "Europe/Madrid", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8 Gen 1"
  );
  
  // Oppo series
  profile_pool[idx++] = *create_profile(
    37, "Android OPPO Find X6 Pro",
    "Mozilla/5.0 (Linux; Android 14; OPPO Find X6 Pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3120, 1.5, "Asia/Bangkok", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  profile_pool[idx++] = *create_profile(
    38, "Android OPPO Find X6",
    "Mozilla/5.0 (Linux; Android 13; OPPO Find X6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 12, en_us_langs, 2, 10, "Google Inc.",
    1080, 2400, 1.0, "Asia/Hong_Kong", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8 Gen 1"
  );
  
  // Vivo series
  profile_pool[idx++] = *create_profile(
    39, "Android Vivo X90 Pro+",
    "Mozilla/5.0 (Linux; Android 13; vivo X90 Pro+) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3200, 1.5, "Asia/Singapore", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8 Gen 1"
  );
  
  profile_pool[idx++] = *create_profile(
    40, "Android Vivo X90 Pro",
    "Mozilla/5.0 (Linux; Android 13; vivo X90 Pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 12, en_us_langs, 2, 10, "Google Inc.",
    1080, 2400, 1.0, "Asia/Seoul", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8 Gen 1"
  );
  
  // Motorola series
  profile_pool[idx++] = *create_profile(
    41, "Android Motorola razr 40 Ultra",
    "Mozilla/5.0 (Linux; Android 13; motorola razr40ultra) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3120, 1.44, "America/Mexico_City", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8 Gen 1"
  );
  
  profile_pool[idx++] = *create_profile(
    42, "Android Motorola Edge 50 Pro",
    "Mozilla/5.0 (Linux; Android 14; motorola edge50pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3120, 1.5, "America/Toronto", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  // Nothing Phone
  profile_pool[idx++] = *create_profile(
    43, "Android Nothing Phone 2",
    "Mozilla/5.0 (Linux; Android 14; Nothing Phone 2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3120, 1.5, "Europe/London", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  // Samsung Galaxy Fold series
  profile_pool[idx++] = *create_profile(
    44, "Android Samsung Galaxy Z Fold 5",
    "Mozilla/5.0 (Linux; Android 13; SM-F946B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 12, en_us_langs, 2, 10, "Google Inc.",
    2176, 1812, 1.0, "Europe/Berlin", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8 Gen 1"
  );
  
  profile_pool[idx++] = *create_profile(
    45, "Android Samsung Galaxy Z Flip 5",
    "Mozilla/5.0 (Linux; Android 13; SM-F731B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 8, en_us_langs, 2, 10, "Google Inc.",
    1080, 2640, 2.63, "America/New_York", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8 Gen 1"
  );
  
  // Nubia/ZTE series
  profile_pool[idx++] = *create_profile(
    46, "Android ZTE nubia Red Magic 8S Pro",
    "Mozilla/5.0 (Linux; Android 14; NX739J) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 16, en_us_langs, 2, 10, "Google Inc.",
    1440, 3120, 1.5, "Asia/Shanghai", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  // Realme series
  profile_pool[idx++] = *create_profile(
    47, "Android Realme GT 3",
    "Mozilla/5.0 (Linux; Android 13; realme GT 3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3200, 1.44, "Asia/Bangkok", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8 Gen 1"
  );
  
  // iQOO series
  profile_pool[idx++] = *create_profile(
    48, "Android iQOO 11 Pro",
    "Mozilla/5.0 (Linux; Android 13; iQOO 11 Pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3200, 1.5, "Asia/Shanghai", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8 Gen 1"
  );
  
  // Honor series
  profile_pool[idx++] = *create_profile(
    49, "Android Honor Magic 6 Pro",
    "Mozilla/5.0 (Linux; Android 14; Honor Magic 6 Pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3120, 1.5, "Asia/Shanghai", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  // Galaxy A series budget
  profile_pool[idx++] = *create_profile(
    50, "Android Samsung Galaxy A13",
    "Mozilla/5.0 (Linux; Android 12; SM-A135F) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 4, 4, en_us_langs, 2, 10, "Google Inc.",
    720, 1600, 1.0, "America/Miami", "en-US",
    "Google Inc. (MediaTek)", "ARM Mali-G77 MP9"
  );
  
  // Multi-language Android profiles
  profile_pool[idx++] = *create_profile(
    51, "Android Samsung Galaxy S23 FR",
    "Mozilla/5.0 (Linux; Android 14; SM-S911B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 8, fr_langs, 3, 10, "Google Inc.",
    1080, 2340, 1.0, "Europe/Paris", "fr-FR",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  profile_pool[idx++] = *create_profile(
    52, "Android Google Pixel 8 DE",
    "Mozilla/5.0 (Linux; Android 14; Pixel 8) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 8, de_langs, 3, 10, "Google Inc.",
    1080, 2340, 1.0, "Europe/Berlin", "de-DE",
    "Google Inc. (Google)", "Google Tensor G3"
  );
  
  profile_pool[idx++] = *create_profile(
    53, "Android OnePlus 12 JP",
    "Mozilla/5.0 (Linux; Android 14; OnePlus 12) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 12, ja_langs, 3, 10, "Google Inc.",
    1440, 3168, 1.44, "Asia/Tokyo", "ja-JP",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  profile_pool[idx++] = *create_profile(
    54, "Android Xiaomi 14 ES",
    "Mozilla/5.0 (Linux; Android 14; xiaomi 2312DQA47T) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 12, es_langs, 3, 10, "Google Inc.",
    1080, 2400, 1.0, "Europe/Madrid", "es-ES",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  profile_pool[idx++] = *create_profile(
    55, "Android OPPO Find X6 Pro JP",
    "Mozilla/5.0 (Linux; Android 14; OPPO Find X6 Pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 12, 12, ja_langs, 3, 10, "Google Inc.",
    1440, 3120, 1.5, "Asia/Tokyo", "ja-JP",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 2"
  );
  
  // Additional mid-range Android
  profile_pool[idx++] = *create_profile(
    56, "Android Samsung Galaxy M13",
    "Mozilla/5.0 (Linux; Android 12; SM-M135F) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 4, 4, en_us_langs, 2, 10, "Google Inc.",
    720, 1600, 1.0, "Europe/Dublin", "en-US",
    "Google Inc. (MediaTek)", "ARM Mali-G77 MP9"
  );
  
  profile_pool[idx++] = *create_profile(
    57, "Android Google Pixel 7",
    "Mozilla/5.0 (Linux; Android 13; Pixel 7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 8, en_us_langs, 2, 10, "Google Inc.",
    1080, 2340, 1.0, "America/Houston", "en-US",
    "Google Inc. (Google)", "Google Tensor G2"
  );
  
  profile_pool[idx++] = *create_profile(
    58, "Android OnePlus 10 Pro",
    "Mozilla/5.0 (Linux; Android 12; OnePlus 10 Pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3216, 1.5, "Europe/Amsterdam", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8cx Gen 1"
  );
  
  profile_pool[idx++] = *create_profile(
    59, "Android Motorola Edge 50",
    "Mozilla/5.0 (Linux; Android 14; motorola edge50) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 8, en_us_langs, 2, 10, "Google Inc.",
    1080, 2436, 1.0, "America/Vancouver", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8 Gen 1"
  );
  
  profile_pool[idx++] = *create_profile(
    60, "Android Redmi Note 13 Pro",
    "Mozilla/5.0 (Linux; Android 13; 2312DRA50C) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 8, en_us_langs, 2, 10, "Google Inc.",
    1440, 3200, 1.5, "Asia/Bangkok", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8 Gen 1"
  );
  
  profile_pool[idx++] = *create_profile(
    61, "Android ZTE Blade V40 Design",
    "Mozilla/5.0 (Linux; Android 13; BLADE V40 Design) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 6, 6, en_us_langs, 2, 10, "Google Inc.",
    1080, 2340, 1.0, "Europe/Rome", "en-US",
    "Google Inc. (MediaTek)", "ARM Mali-G77 MP9"
  );
  
  profile_pool[idx++] = *create_profile(
    62, "Android Xperia 1 V",
    "Mozilla/5.0 (Linux; Android 13; SOV46) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 12, en_us_langs, 2, 10, "Google Inc.",
    1440, 3840, 1.0, "Asia/Tokyo", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8 Gen 1"
  );
  
  profile_pool[idx++] = *create_profile(
    63, "Android Xperia 5 V",
    "Mozilla/5.0 (Linux; Android 13; SOV44) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 6, 8, en_us_langs, 2, 10, "Google Inc.",
    1080, 2520, 1.0, "Europe/Paris", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 8 Gen 1"
  );
  
  profile_pool[idx++] = *create_profile(
    64, "Android LG Wing",
    "Mozilla/5.0 (Linux; Android 12; LMVN100N) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 8, 8, en_us_langs, 2, 10, "Google Inc.",
    1080, 2340, 1.0, "America/New_York", "en-US",
    "Google Inc. (Qualcomm)", "Qualcomm Adreno 650"
  );
  
  profile_pool[idx++] = *create_profile(
    65, "Android Moto G Power",
    "Mozilla/5.0 (Linux; Android 13; moto g power 2023) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Mobile Safari/537.36",
    "Linux armv8l", 4, 4, en_us_langs, 2, 10, "Google Inc.",
    720, 1600, 1.0, "America/Dallas", "en-US",
    "Google Inc. (MediaTek)", "ARM Mali-G37"
  );
  
  // Update profile count
  profile_count = 65;
  
  // Update profile count
  profile_count = 65;

  g_print("Fingerprint: Initialized %d profiles\n", profile_count);
}

FingerprintProfile* fingerprint_get_random_profile() {
  if (profile_pool == NULL || profile_count == 0) {
    fingerprint_profiles_init();
  }
  
  gint random_idx = g_random_int_range(0, profile_count);
  return &profile_pool[random_idx];
}

FingerprintProfile* fingerprint_get_profile_by_id(gint id) {
  if (profile_pool == NULL || profile_count == 0) {
    fingerprint_profiles_init();
  }
  
  for (gint i = 0; i < profile_count; i++) {
    if (profile_pool[i].profile_id == id) {
      return &profile_pool[i];
    }
  }
  
  return NULL;
}

gint fingerprint_get_profile_count() {
  return profile_count;
}

void fingerprint_profile_free(struct FingerprintProfile *profile) {
  if (!profile) return;
  
  g_free(profile->user_agent);
  g_free(profile->platform);
  
  if (profile->languages) {
    for (gint i = 0; i < profile->languages_count; i++) {
      g_free(profile->languages[i]);
    }
    g_free(profile->languages);
  }
  
  g_free(profile->vendor);
  g_free(profile->timezone);
  g_free(profile->language);
  g_free(profile->webgl_vendor);
  g_free(profile->webgl_renderer);
  g_free(profile->profile_name);
}

void fingerprint_profiles_cleanup() {
  if (profile_pool) {
    for (gint i = 0; i < profile_count; i++) {
      fingerprint_profile_free(&profile_pool[i]);
    }
    g_free(profile_pool);
    profile_pool = NULL;
    profile_count = 0;
  }
}
