#include <am.h>
#include <nemu.h>
#include <klib.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
/*	int i;
  int w = 400;  // TODO: get the correct width
  int h = 300;  // TODO: get the correct height
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i ++) fb[i] = i;
  outl(SYNC_ADDR, 1);*/
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
	uint32_t wh = inl(VGACTL_ADDR);
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = wh>>16, .height = wh & 0x0000ffff,
    .vmemsz = 0
  };
  //printf("in __am_gpu_config: width:%d,height:%d\n",cfg->width,cfg->height);
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
	int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;//起点(x,y),以此为起点填充一个h*w的矩形区域
  //printf("x:%d,y:%d,w:%d,h:%d\n",x,y,w,h);
	//if(w==0 || h==0) return;
	uint32_t wh = inl(VGACTL_ADDR);
  uint32_t width = wh >> 16, height = wh&0x0000ffff;
  //printf("in __am_gpu_fbdraw: width:%d,height:%d,sync:%d\n",width,height,ctl->sync);
  //uint32_t height = wh & 0x0000ffff;
  //assert(x>=0 && y>=0 && x+w<=width && y+h<=height);
	//if (x<0 || y<0 || x + w>width || y + h>height) return;
	//写入缓冲区
	uint32_t * fb = (uint32_t*)(uintptr_t)FB_ADDR;
  for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			if(x+j<0 || y+i<0 || x+j>=width || y+i>=height) continue;
			fb[width * (y + i) + (x + j)] = ((uint32_t*)ctl->pixels)[i * w + j];
    }
	}
  if (ctl->sync) {//同步
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
