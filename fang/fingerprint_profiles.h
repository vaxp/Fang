#ifndef FINGERPRINT_PROFILES_H
#define FINGERPRINT_PROFILES_H

#include <glib.h>

// Fingerprint profile structure containing all spoofable attributes
struct FingerprintProfile {
  // Navigator properties
  gchar *user_agent;
  gchar *platform;
  gint hardware_concurrency;
  gint device_memory;
  gchar **languages;
  gint languages_count;
  gint max_touch_points;
  gchar *vendor;
  
  // Screen properties
  gint screen_width;
  gint screen_height;
  gint screen_avail_width;
  gint screen_avail_height;
  gdouble device_pixel_ratio;
  gint color_depth;
  
  // Timezone and locale
  gchar *timezone;
  gchar *language;
  
  // WebGL properties
  gchar *webgl_vendor;
  gchar *webgl_renderer;
  
  // Canvas noise seed (for deterministic noise)
  guint32 canvas_seed;
  
  // Audio noise seed
  guint32 audio_seed;
  
  // Profile metadata
  gchar *profile_name;
  gint profile_id;
};

// Initialize the fingerprint profile system
void fingerprint_profiles_init();

// Get a random profile from the pool
struct FingerprintProfile* fingerprint_get_random_profile();

// Get a specific profile by ID
struct FingerprintProfile* fingerprint_get_profile_by_id(gint id);

// Get total number of profiles
gint fingerprint_get_profile_count();

// Free profile resources
void fingerprint_profile_free(struct FingerprintProfile *profile);

// Cleanup all profiles
void fingerprint_profiles_cleanup();

#endif // FINGERPRINT_PROFILES_H
