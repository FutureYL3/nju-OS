#include <game.h>

static int w, h;

struct ball ball = { .x =0, .y = 0, .v_x = 0, .v_y = 0, .width = 10, .height = 10 };

static void init() {
  AM_GPU_CONFIG_T info = {0};
  ioe_read(AM_GPU_CONFIG, &info);
  w = info.width;
  h = info.height;
}

void kbd_event(int key) {
  switch (key) {
    case AM_KEY_UP:
      ball.v_y += 1;
      break;
    case AM_KEY_DOWN:
      ball.v_y -= 1;
      break;
    case AM_KEY_LEFT:
      ball.v_x -= 1;
      break;
    case AM_KEY_RIGHT:
      ball.v_x += 1;
      break;
    case AM_KEY_ESCAPE:
      halt(0);
  }
}

void game_progress() {
  // update speed
  ball.x += ball.v_x / FPS;
  ball.y += ball.v_y / FPS;

  // handle bump onto the wall
  if (ball.x < 0) {
    ball.x = 0;
    ball.v_x = -ball.v_x;
  }
  if (ball.x >= w) {
    ball.x = w - ball.width - 1;
    ball.v_x = -ball.v_x;
  }
  if (ball.y < 0) {
    ball.y = 0;
    ball.v_y = -ball.v_y;
  }
  if (ball.y >= h) {
    ball.y = h - ball.height - 1;
    ball.v_y = -ball.v_y;
  }
}

// Operating system is a C program!
int main(const char *args) {
  ioe_init();

  init();
  uint64_t next_frame = 0;
  int key = AM_KEY_NONE;
  while (1) {
    while (uptime() < next_frame) ; // 等待一帧的到来
    while ((key = readkey()) != AM_KEY_NONE) {
      kbd_event(key);               // 处理键盘事件
    }
    game_progress();                // 处理一帧游戏逻辑，更新物体的位置等
    screen_update(ball);            // 重新绘制屏幕
    next_frame += 1000 / FPS;       // 计算下一帧的时间
  }
}
