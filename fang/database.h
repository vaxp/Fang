#ifndef DATABASE_H
#define DATABASE_H

#include "types.h"

void initialize_databases(BrowserApp *app);
void add_to_history(BrowserApp *app, const gchar *url, const gchar *title);
void setup_persistent_storage(WebKitWebContext *context);

#endif // DATABASE_H
