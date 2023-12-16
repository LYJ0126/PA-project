#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;//a7
  a[4] = c->GPRx;//a0
  switch (a[0]) {
    case 0:printf("SYS_exit, a0 = %d\n", a[4]); halt(a[4]); break;
    case 1: printf("SYS_yield\n"); yield(); c->GPRx = 0; break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
