#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dir_operations.h"
#include "ui.h"

int main(int argc, char *argv[]) {
  char *input_path;

  if (argc < 2) {

    input_path = getenv("HOME");
    if (input_path == NULL) {

      input_path = ".";
      fprintf(stderr, "Warning: HOME environment variable not set. Defaulting "
                      "to current directory ('.').\n");
    }
    printf("No directory specified. Using default: %s\n", input_path);
  } else {

    input_path = argv[1];
  }

  char resolved_path[MAX_PATH_LEN];

  if (realpath(input_path, resolved_path) == NULL) {
    perror("Error resolving path");
    return EXIT_FAILURE;
  }

  init_ncurses();

  mvprintw(0, 0, "Scanning directory: %s", resolved_path);
  mvprintw(1, 0, "Please wait, this may take a while for large directories...");
  refresh();

  calculate_directory_size(resolved_path, -1, 0);

  if (g_entry_count > 0) {
    g_current_view_index = 0;
  } else {
    cleanup_ncurses();
    fprintf(stderr, "No entries found for '%s'. Exiting.\n", resolved_path);
    return EXIT_FAILURE;
  }

  display_current_view();

  int ch;
  while ((ch = getch()) != 'q') {
    int num_children = 0;
    int *children_indices =
        get_children_indices(g_current_view_index, &num_children);

    qsort(children_indices, num_children, sizeof(int),
          compare_child_indices_by_size);

    switch (ch) {
    case KEY_UP:
      if (g_selected_item_index_in_view > 0) {
        g_selected_item_index_in_view--;
      }
      break;
    case KEY_DOWN:
      if (g_selected_item_index_in_view < num_children - 1) {
        g_selected_item_index_in_view++;
      }
      break;
    case KEY_RIGHT:
      if (num_children > 0 && g_selected_item_index_in_view < num_children) {
        int selected_child_idx =
            children_indices[g_selected_item_index_in_view];
        if (g_entries[selected_child_idx].is_directory) {
          g_current_view_index = selected_child_idx;
          g_selected_item_index_in_view = 0;
        } else {
          show_message("Cannot descend into a file.");
        }
      }
      break;
    case KEY_LEFT:
      if (g_entries[g_current_view_index].parent_index != -1) {
        g_current_view_index = g_entries[g_current_view_index].parent_index;
        g_selected_item_index_in_view = 0;
      } else {
        show_message("Already at root directory.");
      }
      break;
    case 'd':
      if (num_children > 0 && g_selected_item_index_in_view < num_children) {
        int selected_child_idx =
            children_indices[g_selected_item_index_in_view];
        DirEntry *item_to_delete = &g_entries[selected_child_idx];
        char confirm_prompt[MAX_PATH_LEN + 30];
        snprintf(confirm_prompt, sizeof(confirm_prompt),
                 "Delete '%s'? This is permanent!", item_to_delete->path);

        if (confirm_action(confirm_prompt)) {
          int delete_success = 0;
          if (item_to_delete->is_directory) {

            if (rmdir(item_to_delete->path) == 0) {
              delete_success = 1;
              show_message("Directory deleted successfully.");
            } else {
              if (errno == ENOTEMPTY) {
                show_message("Error: Directory is not empty. Cannot delete.");
              } else {
                show_message("Error deleting directory.");
              }
            }
          } else {
            if (remove(item_to_delete->path) == 0) {
              delete_success = 1;
              show_message("File deleted successfully.");
            } else {
              show_message("Error deleting file.");
            }
          }

          if (delete_success) {

            mvprintw(0, 0, "Re-scanning directory: %s", resolved_path);
            mvprintw(1, 0, "Please wait, updating data...");
            refresh();
            reset_g_entries();
            calculate_directory_size(resolved_path, -1, 0);

            if (g_entry_count > 0) {
              g_current_view_index = 0;
              g_selected_item_index_in_view = 0;
            } else {

              show_message("Scan resulted in no entries. Exiting.");
              ch = 'q';
            }
          }
        } else {
          show_message("Deletion cancelled.");
        }
      } else {
        show_message("No item selected for deletion.");
      }
      break;
    }
    free(children_indices);
    display_current_view();
  }

  cleanup_ncurses();

  reset_g_entries();

  printf("\nScan complete. Exiting ncurses mode.\n");

  return EXIT_SUCCESS;
}