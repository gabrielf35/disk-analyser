#ifndef DIR_OPERATIONS_H
#define DIR_OPERATIONS_H

#include <limits.h>

#define MAX_PATH_LEN PATH_MAX

typedef struct {
  char *path;
  long long size;
  int parent_index;
  int is_directory;
  int depth;
} DirEntry;

extern DirEntry *g_entries;
extern int g_entry_count;
extern int g_entry_capacity;

int add_entry(const char *path, long long size, int parent_idx, int is_dir,
              int depth);
long long calculate_directory_size(const char *path, int parent_idx, int depth);
int *get_children_indices(int parent_idx, int *num_children);
void reset_g_entries();

int compare_dir_entries_by_size(const void *a, const void *b);
int compare_child_indices_by_size(const void *a, const void *b);

#endif