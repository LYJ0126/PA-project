#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  printf("IMG_Load: %s\n", filename);
  FILE *fp = fopen(filename, "r");
  assert(fp != NULL);
  fseek(fp, 0, SEEK_END);
  printf("到文件末尾\n");
  long size = ftell(fp);//获取文件大小
  printf("size:%ld\n",size);
  fseek(fp, 0, SEEK_SET);
  printf("回到文件开头\n");
  char *buf = (char*)malloc(size);
  size_t tempsize = fread(buf, 1, (size_t)size, fp);//将整个文件读入buf
  printf("tempsize:%d\n",tempsize);
  assert(tempsize == size);
  SDL_Surface *ret = STBIMG_LoadFromMemory(buf, size);//从内存中加载图片
  printf("加载完成\n");
  fclose(fp);
  free(buf);
  return ret;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
