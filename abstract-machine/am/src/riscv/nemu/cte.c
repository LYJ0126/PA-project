#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  /*for(int i = 0; i < 32; i++){
      printf("%x ", c->gpr[i]);
  }
    printf("\n");
    printf("mcause: %x\n", c->mcause);
    printf("mstatus: %x\n", c->mstatus);
    printf("mepc: %x\n", c->mepc);*/
  if (user_handler) {
    Event ev = {0};
    //printf("c->mcause: %d\n", c->mcause);
    switch (c->mcause) {
      case 0: ev.event = EVENT_SYSCALL; break;
      case 1: ev.event = EVENT_SYSCALL; break;
      case 2: ev.event = EVENT_SYSCALL; break;
      case 3: ev.event = EVENT_SYSCALL; break;
      case 4: ev.event = EVENT_SYSCALL; break;
      case 5: ev.event = EVENT_SYSCALL; break;
      case 6: ev.event = EVENT_SYSCALL; break;
      case 7: ev.event = EVENT_SYSCALL; break;
      case 8: ev.event = EVENT_SYSCALL; break;
      case 9: ev.event = EVENT_SYSCALL; break;
      case 10: ev.event = EVENT_SYSCALL; break;
      case 11: ev.event = EVENT_SYSCALL; break;
      case 12: ev.event = EVENT_SYSCALL; break;
      case 13: ev.event = EVENT_SYSCALL; break;
      case 14: ev.event = EVENT_SYSCALL; break;
      case 15: ev.event = EVENT_SYSCALL; break;
      case 16: ev.event = EVENT_SYSCALL; break;
      case 17: ev.event = EVENT_SYSCALL; break;
      case 18: ev.event = EVENT_SYSCALL; break;
      case 19: ev.event = EVENT_SYSCALL; break;
      case 0xffffffff: ev.event = EVENT_YIELD; break;
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    /*for(int i = 0; i < 32; i++){
      printf("%x ", c->gpr[i]);
  }
    printf("\n");
    printf("mcause: %x\n", c->mcause);
    printf("mstatus: %x\n", c->mstatus);
    printf("mepc: %x\n", c->mepc);*/
    assert(c != NULL);
  }
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));//将异常入口地址设置到mtvec寄存器中。__am_asm_trap是异常处理入口函数,在am-kernels/nexus-am/am/arch/riscv32/src/trap.S中定义

  // register event handler
  user_handler = handler;//注册一个事件处理回调函数,在yield test中,这个回调函数由yield test提供

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  //return NULL;
  Context *c = (Context *)((uint8_t* )kstack.end - sizeof(Context));//kstack.end是栈顶指针,分配一个Context结构体大小的空间
  memset(c, 0, sizeof(Context));//将Context结构体清零
  //将栈顶指针保存在Context记录的sp寄存器对应的位
  //c->gpr[2] = (uintptr_t)kstack.end;
  //设置内核线程参数
  c-> GPR2 = (uintptr_t)arg;
  //设置内核线程入口
  c->mepc = (uintptr_t)entry;
  return c;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");//ecall触发一个系统调用,转到异常入口地址mtvec寄存器中保存的地址处执行
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
