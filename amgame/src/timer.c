#include <game.h>

uint64_t uptime() {
  return io_read(AM_TIMER_UPTIME).us / 1000;
}