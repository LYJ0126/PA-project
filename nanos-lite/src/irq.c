#include <common.h>
void do_syscall(Context*);

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    case EVENT_YIELD:printf("Event yield, event ID = %d\n", e.event); break;
    case EVENT_SYSCALL:
    printf("Event syscall, event ID = %d\n", e.event);
    printf("a7:c->GPR1,a0:c->GPR2,a1:c->GPR3,a2:c->GPR4,a0:c->GPRx\n");
    do_syscall(c);
    printf("a7:c->GPR1,a0:c->GPR2,a1:c->GPR3,a2:c->GPR4,a0:c->GPRx\n");
    break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
