#include <common.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for(int i = 0; i < len; ++i){
    putch(((char *)buf)[i]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  //return 0;
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if(ev.keycode == AM_KEY_NONE) {//没有按键按下,返回0
    *(char *)buf = '\0';
    return 0;
  }
  bool down = ev.keydown;
  int ret;
  if(down) {//按键按下
    ret = sprintf(buf, "kd %s\n", keyname[ev.keycode]);
  }
  else {//按键弹起
    ret = sprintf(buf, "ku %s\n", keyname[ev.keycode]);
  }
  return (size_t)ret;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

int mygettimeofday(struct timeval *tv, struct timezone *tz) {
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  if(tv != NULL) tv->tv_sec = us / 1000000, tv->tv_usec = us % 1000000;
  if(tz != NULL){}
  return 0;
}
void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
