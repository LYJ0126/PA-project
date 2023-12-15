#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;//a7

  switch (a[0]) {
    //case 0:printf("SYS_exit, a0 = %d\n", c->GPR2); halt(c->GPR2); break;
    case 1: printf("SYS_yield\n"); yield(); c->GPRx = 0; printf("end of yield\n"); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
