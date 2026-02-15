#include <game.h>

uint64_t uptime() {
  uint64_t time = io_read(AM_TIMER_UPTIME).us / 1000;
  printf("[uptime] get time: %d\n", time);
  return time;
}