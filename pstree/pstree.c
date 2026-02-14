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

int compare_pids(const void *a, const void *b) {
  pid_t pid_a = *(const pid_t *)a;
  pid_t pid_b = *(const pid_t *)b;

  return (int)(pid_a - pid_b);
}

void printf_with_pid(int depth, char *name, pid_t pid) {
  for (int i = 0; i < depth; ++i) {
    fprintf(stdout, "  ");
  }
  fprintf(stdout, "%s(%d)\n", name, (int)pid);
}

void printf_without_pid(int depth, char *name) {
  for (int i = 0; i < depth; ++i) {
    fprintf(stdout, "  ");
  }
  fprintf(stdout, "%s\n", name);
}

FILE* get_process_status(pid_t pid) {
  char path[256];
  snprintf(path, sizeof(path), "%s/%d/%s", "/proc", (int)pid, "status");
  FILE* file;
  file = fopen(path, "r");
  if (file == NULL) {
    perror("cannot open status file");
    // exit(EXIT_FAILURE);  do not just exit for robustness
    return NULL;
  }
  return file;
}

char* get_process_name(pid_t pid) {
  FILE* file = get_process_status(pid);
  if (file == NULL) return NULL;

  char line[256];
  char temp_name[256];
  char *name = NULL;
  while (fgets(line, sizeof(line), file)) {
    if (strncmp(line, "Name:", 5) == 0) {
      if (sscanf(line, "Name: %s", temp_name) == 1) {
        name = strdup(temp_name);
        break;
      }
    } 
  }
  fclose(file); 
  return name;
}

void print_children_process(int *depth, pid_t pid, bool show_pid, bool sort) {
  char *name = get_process_name(pid);
  if (name != NULL) {
    if (show_pid) printf_with_pid(*depth, name, pid);
    else          printf_without_pid(*depth, name);

    free(name);
  }
  else  /* exit(EXIT_FAILURE); */ return;
  
  char children_path[256];
  snprintf(children_path, sizeof(children_path), "%s/%d/%s/%d/%s", "/proc", (int)pid, "task", (int)pid, "children");
  FILE* children;
  children = fopen(children_path, "r");
  if (children == NULL) {
    perror("fopen");
    // exit(EXIT_FAILURE);
    return;
  }
  int child_pid;
  int children_count = 0;
  if (!sort) {
    while (fscanf(children, "%d", &child_pid) == 1) {
      // start recursion
      (*depth)++;
      print_children_process(depth, (pid_t)child_pid, show_pid, sort);
    }
  }
  else {
    // get children count
    while (fscanf(children, "%d", &child_pid) == 1) {
      children_count++;
    }
    pid_t *children_pids = (pid_t*)malloc(sizeof(pid_t) * children_count);
    rewind(children);
    // collect children pid
    for (int i = 0; i < children_count; ++i) {
      fscanf(children, "%d", &child_pid);
      children_pids[i] = child_pid;
    }
    // sort children by pid
    qsort(children, children_count, sizeof(pid_t), compare_pids);
    // recursively print child process
    (*depth)++;
    for (int i = 0; i < children_count; ++i) {
      print_children_process(depth, children_pids[i], show_pid, sort);
    }
  }
  
  (*depth)--;
}

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
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
    fprintf(stderr, "pstree (for NJU-OS) 1.0\nCopyright (C) 2026 yl33\n");
    exit(EXIT_SUCCESS);
  }

  // recursively print child process, start from pid 1
  int depth = 0;
  print_children_process(&depth, 1, show_pid, sort);

  exit(EXIT_SUCCESS);
}
