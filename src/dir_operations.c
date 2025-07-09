#define _POSIX_C_SOURCE 200809L

#include "dir_operations.h"
#include "ui.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

DirEntry *g_entries = NULL;
int g_entry_count = 0;
int g_entry_capacity = 0;

int add_entry(const char *path, long long size, int parent_idx, int is_dir,
              int depth) {

  if (g_entry_count >= g_entry_capacity) {
    g_entry_capacity = (g_entry_capacity == 0) ? 1000 : g_entry_capacity * 2;
    g_entries =
        (DirEntry *)realloc(g_entries, g_entry_capacity * sizeof(DirEntry));
    if (g_entries == NULL) {
      perror("realloc failed in add_entry");

      endwin();
      exit(EXIT_FAILURE);
    }
  }

  g_entries[g_entry_count].path = (char *)malloc(strlen(path) + 1);
  if (g_entries[g_entry_count].path == NULL) {
    perror("malloc failed for path in add_entry");
    endwin();
    exit(EXIT_FAILURE);
  }
  strcpy(g_entries[g_entry_count].path, path);
  g_entries[g_entry_count].size = size;
  g_entries[g_entry_count].parent_index = parent_idx;
  g_entries[g_entry_count].is_directory = is_dir;
  g_entries[g_entry_count].depth = depth;

  return g_entry_count++;
}

long long calculate_directory_size(const char *path, int parent_idx,
                                   int depth) {
  DIR *dir;
  struct dirent *entry;
  struct stat statbuf;
  long long total_size = 0;

  int current_dir_entry_idx = add_entry(path, 0, parent_idx, 1, depth);

  dir = opendir(path);
  if (dir == NULL) {

    fprintf(stderr, "Error: Could not open directory '%s' (%s)\n", path,
            strerror(errno));
    g_entries[current_dir_entry_idx].size = 0;
    return 0;
  }

  while ((entry = readdir(dir)) != NULL) {

    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    char full_path[MAX_PATH_LEN];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

    if (lstat(full_path, &statbuf) == -1) {
      fprintf(stderr, "Error: Could not get status of '%s' (%s)\n", full_path,
              strerror(errno));
      continue;
    }

    if (S_ISDIR(statbuf.st_mode)) {
      total_size +=
          calculate_directory_size(full_path, current_dir_entry_idx, depth + 1);
    }

    else if (S_ISREG(statbuf.st_mode)) {
      total_size += statbuf.st_size;
      add_entry(full_path, statbuf.st_size, current_dir_entry_idx, 0,
                depth + 1);
    }
  }

  closedir(dir);

  g_entries[current_dir_entry_idx].size = total_size;

  return total_size;
}

int *get_children_indices(int parent_idx, int *num_children) {
  int *children = NULL;
  int capacity = 0;
  *num_children = 0;

  for (int i = 0; i < g_entry_count; i++) {
    if (g_entries[i].parent_index == parent_idx) {
      if (*num_children >= capacity) {
        capacity = (capacity == 0) ? 10 : capacity * 2;
        children = (int *)realloc(children, capacity * sizeof(int));
        if (children == NULL) {
          perror("realloc failed for children in get_children_indices");
          endwin();
          exit(EXIT_FAILURE);
        }
      }
      children[(*num_children)++] = i;
    }
  }
  return children;
}

void reset_g_entries() {
  for (int i = 0; i < g_entry_count; i++) {
    free(g_entries[i].path);
  }
  free(g_entries);
  g_entries = NULL;
  g_entry_count = 0;
  g_entry_capacity = 0;
}

int compare_dir_entries_by_size(const void *a, const void *b) {
  const DirEntry *entry_a = (const DirEntry *)a;
  const DirEntry *entry_b = (const DirEntry *)b;
  if (entry_a->size < entry_b->size)
    return 1;
  if (entry_a->size > entry_b->size)
    return -1;
  return 0;
}

int compare_child_indices_by_size(const void *a, const void *b) {
  int idx_a = *(const int *)a;
  int idx_b = *(const int *)b;
  if (g_entries[idx_a].size < g_entries[idx_b].size)
    return 1;
  if (g_entries[idx_a].size > g_entries[idx_b].size)
    return -1;
  return 0;
}