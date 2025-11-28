#ifndef ADBLOCKER_H
#define ADBLOCKER_H

#include "types.h"

void adblocker_init(BrowserApp *app);
void adblocker_enable(BrowserApp *app, gboolean enable);
void privacy_enable(BrowserApp *app, gboolean enable);
void apply_privacy_settings(WebKitWebView *web_view, BrowserApp *app);

#endif // ADBLOCKER_H
