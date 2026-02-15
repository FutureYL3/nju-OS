#ifndef GAME_H
#define GAME_H

#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <klib-macros.h>

#define FPS 60

struct ball {
  int x, y, v_x, v_y, width, height;
};

extern struct ball ball;

void splash();
void print_key();
static inline void puts(const char *s) {
  for (; *s; s++) putch(*s);
}
uint64_t uptime();
int readkey();
void kbd_event(int key);
void game_progress();
void screen_update(struct ball ball);

#endif