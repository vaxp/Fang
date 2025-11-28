#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include "types.h"

void show_bookmarks_window(GtkMenuItem *item, BrowserApp *app);
void on_add_bookmark(GtkMenuItem *item, BrowserApp *app);
void on_bookmark_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, BrowserApp *app);

#endif // BOOKMARKS_H
