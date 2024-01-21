#include <am.h>

Area heap;
extern int write(int fd,const void *buf,size_t cnt);
void putch(char ch) {
    write(1, (void*)&ch, 1);//利用系统调用进行输出
}

void halt(int code) {
    exit(code);//利用系统调用进行退出
}