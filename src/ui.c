#include "dir_operations.h"
#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int g_current_view_index = -1;
int g_selected_item_index_in_view = 0;

void init_ncurses() {
  initscr();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);
}

void cleanup_ncurses() { endwin(); }

const char *format_size(long long bytes) {
  static char buffer[32];
  double size = (double)bytes;
  const char *units[] = {"B", "KB", "MB", "GB", "TB"};
  int i = 0;
  while (size >= 1024 && i < 4) {
    size /= 1024;
    i++;
  }
  snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[i]);
  return buffer;
}

void show_message(const char *message) {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  int win_height = 3;
  int msg_len = strlen(message);
  int win_width = msg_len + 4;
  if (win_width > max_x - 4)
    win_width = max_x - 4;
  int start_y = (max_y - win_height) / 2;
  int start_x = (max_x - win_width) / 2;

  WINDOW *msg_win = newwin(win_height, win_width, start_y, start_x);
  box(msg_win, 0, 0);
  mvwprintw(msg_win, 1, 2, "%s", message);
  wrefresh(msg_win);
  napms(1500);
  delwin(msg_win);
  touchwin(stdscr);
  refresh();
}

int confirm_action(const char *prompt) {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);

  int win_height = 5;
  int prompt_len = strlen(prompt);
  int win_width = prompt_len + 10;
  if (win_width > max_x - 4)
    win_width = max_x - 4;
  int start_y = (max_y - win_height) / 2;
  int start_x = (max_x - win_width) / 2;

  WINDOW *confirm_win = newwin(win_height, win_width, start_y, start_x);
  box(confirm_win, 0, 0);
  mvwprintw(confirm_win, 1, 2, "%s", prompt);
  mvwprintw(confirm_win, 3, 2, "Confirm (y/n): ");
  wrefresh(confirm_win);

  int ch = wgetch(confirm_win);
  delwin(confirm_win);
  touchwin(stdscr);
  refresh();

  return (ch == 'y' || ch == 'Y');
}

void display_current_view() {
  clear();

  if (g_current_view_index == -1) {
    mvprintw(0, 0, "Error: No directory selected for view.");
    refresh();
    return;
  }

  DirEntry *current_dir = &g_entries[g_current_view_index];
  mvprintw(0, 0, "Current Directory: %s (Total Size: %s)", current_dir->path,
           format_size(current_dir->size));
  mvprintw(1, 0,
           "-------------------------------------------------------------------"
           "-------------");
  mvprintw(LINES - 2, 0,
           "Use Up/Down arrows to navigate, Right to descend, Left to ascend, "
           "'d' to delete, 'q' to quit.");

  int num_children = 0;
  int *children_indices =
      get_children_indices(g_current_view_index, &num_children);

  qsort(children_indices, num_children, sizeof(int),
        compare_child_indices_by_size);

  int start_row = 3;
  int max_rows = LINES - start_row - 3;

  int scroll_offset = 0;
  if (g_selected_item_index_in_view >= max_rows && max_rows > 0) {
    scroll_offset = g_selected_item_index_in_view - max_rows + 1;
  }

  for (int i = 0; i < num_children; i++) {
    if (i - scroll_offset >= max_rows)
      break;
    if (i < scroll_offset)
      continue;

    int child_idx = children_indices[i];
    DirEntry *child_entry = &g_entries[child_idx];

    if (i == g_selected_item_index_in_view) {
      attron(A_REVERSE);
    }

    const char *name = strrchr(child_entry->path, '/');
    if (name) {
      name++;
    } else {
      name = child_entry->path;
    }

    mvprintw(start_row + (i - scroll_offset), 0, "%-10s %s %s",
             format_size(child_entry->size),
             child_entry->is_directory ? "[DIR]" : "     ", name);
    if (i == g_selected_item_index_in_view) {
      attroff(A_REVERSE);
    }
  }

  free(children_indices);
  refresh();
}