#ifndef HISTORY_H
#define HISTORY_H

#include "types.h"

void show_history_window(GtkMenuItem *item, BrowserApp *app);
void on_history_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, BrowserApp *app);

#endif // HISTORY_H
