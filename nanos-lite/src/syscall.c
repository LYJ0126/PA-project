#include <common.h>
#include <sys/time.h>
#include <time.h>
#include "syscall.h"
extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_write(int fd, const void *buf, size_t len);
extern size_t fs_lseek(int fd, size_t offset, int whence);
extern int fs_close(int fd);
extern int mygettimeofday(struct timeval *tv, struct timezone *tz);
extern int execve(const char *pathname, char *const argv[],char *const envp[]);
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;//a7
  a[1] = c->GPR2;//a0
  a[2] = c->GPR3;//a1
  a[3] = c->GPR4;//a2
  a[4] = c->GPRx;//a0
  switch (a[0]) {
    case 0: {
      //printf("SYS_exit, a0 = %d\n", a[1]); 
      execve("/bin/nterm", NULL, NULL);
      break;
    }
    case 1: printf("SYS_yield\n"); yield(); c->GPRx = 0; break;
    case 2: {
      //printf("SYS_open, a0 = %s, a1 = %d\n", a[1], a[2]); 
      c->GPRx = fs_open((char *)a[1], a[2], 0); 
      break;
    }
    case 3: {
    //printf("SYS_read, a0 = %d, a1 = %x, a2 = %d\n", a[1], a[2], a[3]); 
    c->GPRx = fs_read(a[1], (void *)a[2], a[3]); 
    break;
    }
    case 4: {
      //printf("SYS_write, a0 = %d, a1 = %x, a2 = %d\n", a[1], a[2], a[3]); 
      c->GPRx = fs_write(a[1], (void *)a[2], a[3]); 
      break;
    }
    /*case 4:{ 
      printf("SYS_write, a0 = %d, a1 = %x, a2 = %d\n", a[1], a[2], a[3]);
      int flag = 0;
      if(a[1] == 1 || a[1] == 2) {//stdout or stderr
        char *buf = (char *)a[2];
        int i = 0;
        for(; i < a[3]; i++) {//a[3]是写入的字节数
          putch(buf[i]);
        }
        if(i == a[3]) flag = 1;
      }
      if(flag) c->GPRx = a[3];
      else c->GPRx = -1;
      break;
    }*/
    case 7: {
      //printf("SYS_close, a0 = %d\n", a[1]); 
      c->GPRx = fs_close(a[1]); 
      break;
    }
    case 8: {
      //printf("SYS_lseek, a0 = %d, a1 = %d, a2 = %d\n", a[1], a[2], a[3]); 
      c->GPRx =fs_lseek(a[1], a[2], a[3]); 
      break;
    }
    case 9: c->GPRx = 0; break;
    case 13: {
      //printf("SYS_execve, a0 = %s, a1 = %x, a2 = %x\n", a[1], a[2], a[3]);
      c->GPRx = execve((const char *)a[1], (char **const)a[2], (char **const)a[3]);
      break;
    }
    case 19: {
      //printf("SYS_gettimeofday, a0 = %x, a1 = %x\n", a[1], a[2]);
      struct timeval *tv = (struct timeval *)a[1];
      struct timezone *tz = (struct timezone *)a[2];
      c->GPRx = mygettimeofday(tv, tz);
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
