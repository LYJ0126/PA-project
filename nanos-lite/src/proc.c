#include <proc.h>

extern void naive_uload(PCB *pcb, const char *filename);
extern int fs_open(const char *pathname, int flags, int mode);
extern void context_uload(PCB *pcb, const char *filename);
#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;


void context_kload(PCB*pcb, void (*entry)(void *), void *arg) {
  Area kstack;
  kstack.start = (void *)pcb;
  kstack.end = (void *)((uint8_t* )pcb + STACK_SIZE);
  //printf("kstack.start:%x,kstack.end:%x\n",kstack.start,kstack.end);
  Context *c = kcontext(kstack, entry, arg);
  pcb->cp = c;
  //printf("pcb->cp:%x\n",pcb->cp);
}

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  while (1) {
    putch("?CD"[(uintptr_t)arg > 2 ? 0 : (uintptr_t)arg]);
    for (int volatile i = 0; i < 500000; i++) ;
    yield();
  }
  /*int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }*/
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)1L);
  //context_kload(&pcb[1], hello_fun, (void *)2L);
  context_uload(&pcb[1], "/bin/pal");
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  //printf("start naive_uload\n");
  //naive_uload(NULL, "/bin/nterm");
  //naive_uload(NULL, "/bin/dummy");
  //printf("end naive_uload\n");
}

Context* schedule(Context *prev) {
  //return NULL;
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}

int execve(const char *pathname, char *const argv[],char *const envp[]){
  int fd = fs_open(pathname, 0, 0);
  if(fd == -1) return -1;
  //printf("fd:%d\n",fd);
  //printf("pathname:%s\n",pathname);
  naive_uload(NULL, pathname);
  return 0;
}
