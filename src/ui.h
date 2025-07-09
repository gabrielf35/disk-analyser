#ifndef UI_H
#define UI_H

#include <ncurses.h>

extern int g_current_view_index;
extern int g_selected_item_index_in_view;

void init_ncurses();
void cleanup_ncurses();
void display_current_view();
void show_message(const char *message);
int confirm_action(const char *prompt);
const char *format_size(long long bytes);

#endif