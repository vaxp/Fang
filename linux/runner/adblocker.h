#ifndef ADBLOCKER_H
#define ADBLOCKER_H

#include "types.h"

void adblocker_init(BrowserApp *app);
void adblocker_enable(BrowserApp *app, gboolean enable);
gboolean adblocker_reload_filter(BrowserApp *app);
void privacy_enable(BrowserApp *app, gboolean enable);
void apply_privacy_settings(WebKitWebView *web_view, BrowserApp *app);

// Fingerprint management
void fingerprint_init(BrowserApp *app);
void fingerprint_rotate_profile(BrowserApp *app);
void fingerprint_apply_to_webview(WebKitWebView *web_view, BrowserApp *app);
void fingerprint_cleanup(BrowserApp *app);

#endif // ADBLOCKER_H
