#ifndef PRIVACY_SCRIPT_H
#define PRIVACY_SCRIPT_H

#include "fingerprint_profiles.h"

// Generate comprehensive anti-fingerprinting JavaScript for a given profile
gchar* generate_privacy_script(struct FingerprintProfile *profile);

// Free generated script
void free_privacy_script(gchar *script);

#endif // PRIVACY_SCRIPT_H
