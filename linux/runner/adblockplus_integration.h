#ifndef ADBLOCKPLUS_INTEGRATION_H
#define ADBLOCKPLUS_INTEGRATION_H

#include <glib.h>

// Initialize AdblockPlus integration with EasyList rules
void adblockplus_init();

// Check if URL should be blocked according to EasyList
gboolean adblockplus_should_block_url(const char *url);

// Get array of CSS selectors for ad hiding
const char** adblockplus_get_ad_hiding_selectors();

// Get total number of blocking rules
guint adblockplus_get_rule_count();

// Get total number of CSS selectors
guint adblockplus_get_selector_count();

// Generate complete CSS filter string
gchar* adblockplus_generate_css_filter();

#endif // ADBLOCKPLUS_INTEGRATION_H
