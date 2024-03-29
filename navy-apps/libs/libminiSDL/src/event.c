#include <NDL.h>
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

static uint8_t keystate[sizeof(keyname)/sizeof(keyname[0])] = {0};
int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[20];
  memset(buf, 0, sizeof(buf));
  if(NDL_PollEvent(buf, 20) == 0) return 0;//没有事件发生,立即返回
  if(buf[0]=='k' && buf[1]=='d') ev->type = SDL_KEYDOWN;
  else if(buf[0]=='k' && buf[1]=='u') ev->type = SDL_KEYUP;
  int len = sizeof(keyname) / sizeof(keyname[0]);
  for(int i = 0; i < len; i++){
    if(strlen(keyname[i]) == strlen(buf + 3) - 1){//注意buf+3的长度比keyname[i]的长度多1
      if(strncmp(keyname[i], buf + 3, strlen(keyname[i])) == 0){
        ev->key.keysym.sym = i;
        keystate[i] = (ev->type == SDL_KEYDOWN ? 1 : 0);
        break;
      }
    }
  }
  return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[20];
  memset(buf, 0, sizeof(buf));
  while(NDL_PollEvent(buf, 20) == 0);
  //printf("buf:%s\n",buf);
  if(buf[0]=='k' && buf[1]=='d') event->type = SDL_KEYDOWN;
  else if(buf[0]=='k' && buf[1]=='u') event->type = SDL_KEYUP;
  int len = sizeof(keyname) / sizeof(keyname[0]);
  //printf("len:%d\n",len);
  /*for(int i = 0; i < len; i++){
    printf("keyname[%d]:%s\n",i,keyname[i]);
  }*/
  for(int i = 0; i < len; i++){
    /*printf("keyname[%d]:%s\n",i,keyname[i]);
    printf("buf + 3:%s\n",buf + 3);
    printf("strlen(keyname[i]):%d\n",strlen(keyname[i]));
    printf("strlen(buf + 3):%d\n",strlen(buf + 3));
    printf("strcmp(keyname[i], buf + 3):%d\n",strcmp(keyname[i], buf + 3));*/
    if(strlen(keyname[i]) == strlen(buf + 3) - 1){//注意buf+3的长度比keyname[i]的长度多1
      if(strncmp(keyname[i], buf + 3, strlen(keyname[i])) == 0){
        event->key.keysym.sym = i;
        //printf("event->key.keysym.sym:%d\n",event->key.keysym.sym);
        keystate[i] = (event->type == SDL_KEYDOWN ? 1 : 0);
        break;
      }
    }
  }
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return keystate;
}
