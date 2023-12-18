#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

static int evtdev = -1;
static int fbdev = -1;
static int dispinfodev = -1;
static int screen_w = 0, screen_h = 0;

//extern int mygettimeofday(struct timeval *tv, struct timezone *tz);
struct timeval *tv;
uint32_t NDL_GetTicks() {
  struct timeval temptv;
  gettimeofday(&temptv, NULL);
  return (temptv.tv_sec - tv->tv_sec) * 1000 + (temptv.tv_usec - tv->tv_usec) / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  //return 0;
  buf[0] = '\0';
  int fd = open("/dev/events", 0, 0);
  return read(fd, buf, len);
}

void NDL_OpenCanvas(int *w, int *h) {
  if(*w == 0 && *h == 0) {
    *w = screen_w;
    *h = screen_h;
  }
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
  /*int fd = open("/proc/dispinfo", 0, 0);
  char buf[128];
  read(fd, buf, sizeof(buf));
  sscanf(buf, "WIDTH: %d\nHEIGHT: %d", &screen_w, &screen_h);
  if(*w == 0 && *h == 0) {
    *w = screen_w;
    *h = screen_h;
  }
  if(*w > screen_w) *w = screen_w;
  if(*h > screen_h) *h = screen_h;
  printf("screen_w = %d, screen_h = %d\n", screen_w, screen_h);
  //screen_w = *w; screen_h = *h;
  close(fd);*/
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  
  int fd = open("/dev/fb", 0, 0);
  for(int i=0;i<h;++i){
    lseek(fd, (x + (y + i) * screen_w) * sizeof(uint32_t), SEEK_SET);
    write(fd, pixels + i * w, w * sizeof(uint32_t));
  }
  //close(fd);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  tv = (struct timeval *)malloc(sizeof(struct timeval));
  //mygettimeofday(tv, NULL);
  gettimeofday(tv, NULL);
  int fd = open("/proc/dispinfo", 0, 0);
  char buf[128];
  printf("fd:%d\n",fd);
  read(fd, buf, sizeof(buf));
  printf("buf:%s\n",buf);
  sscanf(buf, "WIDTH:%d\nHEIGHT:%d", &screen_w, &screen_h);
  printf("screen_w = %d, screen_h = %d\n", screen_w, screen_h);
  /*
  FILE *fp = fopen("/proc/dispinfo", "r");
  fscanf(fp, "WIDTH:%d\nHEIGHT:%d", &screen_w, &screen_h);
  printf("screen_w = %d, screen_h = %d\n", screen_w, screen_h);
  //fclose(fp);
  //close(fd);
  */
  return 0;
}

void NDL_Quit() {
  free(tv);
}
