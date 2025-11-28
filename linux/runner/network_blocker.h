#ifndef NETWORK_BLOCKER_H
#define NETWORK_BLOCKER_H

#include "types.h"
#include <webkit2/webkit2.h>

// Initialize network-level blocking
void network_blocker_init(BrowserApp *app);

// Request interception callback
gboolean on_send_request(WebKitWebView *web_view, WebKitURIRequest *request,
                         WebKitURIResponse *redirected_response, gpointer user_data);

// Check if request should be blocked
gboolean should_block_request(const char *uri);

// Get blocking statistics
guint64 get_blocked_requests_count(BrowserApp *app);

#endif // NETWORK_BLOCKER_H
