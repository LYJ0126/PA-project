#include <common.h>
void do_syscall(Context*);

static Context* do_event(Event e, Context* c) {
  //printf("event ID = %d\n", e.event);
  switch (e.event) {
    case EVENT_YIELD:printf("Event yield, event ID = %d\n", e.event); break;
    case EVENT_SYSCALL:
    //printf("Event syscall, event ID = %d\n", e.event);
    //printf("a7:%d,a0:%d,a1:%d,a2:%d,a0:%d\n",c->GPR1,c->GPR2,c->GPR3,c->GPR4,c->GPRx);
    do_syscall(c);
    //printf("a7:%d,a0:%d,a1:%d,a2:%d,a0:%d\n",c->GPR1,c->GPR2,c->GPR3,c->GPR4,c->GPRx);
    break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
