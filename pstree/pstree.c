#define _GNU_SOURCE

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

typedef struct process_info {
  char* name;
  pid_t pid;
  pid_t ppid;
  int child_thread_num;
  struct process_info** child_threads; // pointer to the child threads array 
} process_info;

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);

  // command line arguments collection
  bool show_pid = false, sort = false, version = false;
  int opt;
  while ((opt = getopt(argc, argv, "pnV")) != -1) {
    switch (opt) {
      case 'p':
        show_pid = true;
        break;
      case 'n':
        sort = true;
        break;
      case 'V':
        version = true;
        break;
      case '?':
        fprintf(stderr, "Usage: %s [-V] [-n] [-p]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }
  // if only print version, just print it and early return
  if (version) {
    fprintf(stderr, "pstree (for NJU-OS) 1.0\nCopyright (C) 2026 yl\n");
    exit(EXIT_SUCCESS);
  }
  // collect process info
  DIR *dir;
  struct dirent *entry;

  // Open the directory
  dir = opendir("/proc");
  if (dir == NULL) {
    perror("opendir");
    exit(EXIT_FAILURE);
  }

  // Read directory entries
  while ((entry = readdir(dir)) != NULL) {
    // Check if the entry is a directory and not "." or ".."
    if (entry->d_type == DT_DIR) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
          printf("%s\n", entry->d_name);
        }
    }
  }

  // Close the directory
  closedir(dir);
  return 0;
}
