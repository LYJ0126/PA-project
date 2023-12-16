#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;//a7
  a[1] = c->GPR2;//a0
  a[2] = c->GPR3;//a1
  a[3] = c->GPR4;//a2
  a[4] = c->GPRx;//a0
  switch (a[0]) {
    case 0:printf("SYS_exit, a0 = %d\n", a[4]); halt(a[4]); break;
    case 1: printf("SYS_yield\n"); yield(); c->GPRx = 0; break;
    case 4:{ 
      printf("SYS_write, a0 = %d, a1 = %x, a2 = %d\n", a[1], a[2], a[3]);
      if(a[1] == 1 || a[1] == 2) {//stdout or stderr
        char *buf = (char *)a[2];
        for(int i = 0; i < a[3]; i++) {//a[3]是写入的字节数
          putch(buf[i]);
        }
      }
      c->GPRx = a[3];
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
