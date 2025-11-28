#ifndef TABS_H
#define TABS_H

#include "types.h"

BrowserTab* create_new_tab(BrowserApp *app, const gchar *uri);
void close_tab(BrowserApp *app, BrowserTab *tab);
void switch_to_tab(BrowserApp *app, BrowserTab *tab);
void update_url_bar(BrowserApp *app, BrowserTab *tab);

// Callbacks
void on_tab_switched(GtkNotebook *notebook, GtkWidget *page, guint page_num, BrowserApp *app);
void on_uri_changed(WebKitWebView *web_view, GParamSpec *spec, BrowserApp *app);
void on_title_changed(WebKitWebView *web_view, GParamSpec *spec, BrowserApp *app);
void on_load_changed(WebKitWebView *web_view, WebKitLoadEvent load_event, BrowserApp *app);
void on_close_tab_clicked(GtkButton *button, BrowserApp *app);
gboolean on_decide_policy(WebKitWebView *web_view, WebKitPolicyDecision *decision, WebKitPolicyDecisionType decision_type, BrowserApp *app);
gboolean on_permission_request(WebKitWebView *web_view, WebKitPermissionRequest *request, BrowserApp *app);
gboolean on_enter_fullscreen(WebKitWebView *web_view, BrowserApp *app);
gboolean on_leave_fullscreen(WebKitWebView *web_view, BrowserApp *app);
void on_download_started(WebKitWebContext *context, WebKitDownload *download, BrowserApp *app);

#endif // TABS_H
