#include <game.h>

#define SIDE 16
static int w, h;
static bool initilized = false;

static void init() {
  AM_GPU_CONFIG_T info = {0};
  ioe_read(AM_GPU_CONFIG, &info);
  w = info.width;
  h = info.height;
}

static void draw_tile(int x, int y, int w, int h, uint32_t color, int sync) {
  uint32_t pixels[w * h]; // WARNING: large stack-allocated memory
  AM_GPU_FBDRAW_T event = {
    .x = x, .y = y, .w = w, .h = h, .sync = sync,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  ioe_write(AM_GPU_FBDRAW, &event);
}

void splash() {
  init();
  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if ((x & 1) ^ (y & 1)) {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff, 1); // white
      }
    }
  }
}

void screen_update(struct ball ball) {
  printf("[screen update] start screen update\n");
  if (!initilized) {
    init();
    initilized = true;
  }

  // clear the screen
  for (int x = 0; x * SIDE < w; x ++) {
    for (int y = 0; y * SIDE < h; y++) {
      draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0x000000, 0); // white
    }
  }

  // draw the ball
  uint32_t pixels[ball.width * ball.height];
  AM_GPU_FBDRAW_T event = {
    .x = ball.x, .y = ball.y, .w = ball.width, .h = ball.height, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < ball.width * ball.height; ++ i) {
    pixels[i] = 0xffffff; // white ball
  }

  ioe_write(AM_GPU_FBDRAW, &event);

  printf("[screen update] screen updated\n");
}
