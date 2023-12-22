#include <proc.h>

extern void naive_uload(PCB *pcb, const char *filename);
extern int fs_open(const char *pathname, int flags, int mode);
#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  //printf("start naive_uload\n");
  naive_uload(NULL, "/bin/nterm");
  //printf("end naive_uload\n");
}

Context* schedule(Context *prev) {
  return NULL;
}

int execve(const char *pathname, char *const argv[],char *const envp[]){
  int fd = fs_open(pathname, 0, 0);
  if(fd == -1) return -1;
  //printf("fd:%d\n",fd);
  //printf("pathname:%s\n",pathname);
  naive_uload(NULL, pathname);
  return 0;
}
