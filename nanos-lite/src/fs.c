#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);
extern size_t serial_write(const void *buf, size_t offset, size_t len);
extern size_t events_read(void *buf, size_t offset, size_t len);
typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  int open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write, 0},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write, 0},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write, 0},
#include "files.h"
};
#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

int fs_open(const char *pathname, int flags, int mode) {
  //nanos中没有用到flags和mode
  for(int i = 3; i < NR_FILES; i++){
    if(strcmp(file_table[i].name, pathname) == 0){
      file_table[i].open_offset = 0;//打开文件时，将文件偏移量置为0
      printf("open file:%s\n",pathname);
      return i;
    }
  }
  panic("file not found");
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  /*if(fd < 3) return 0;
  if(file_table[fd].open_offset + len > file_table[fd].size){
    len = file_table[fd].size - file_table[fd].open_offset;
  }
  file_table[fd].read = ramdisk_read;//文件读操作用ramdisk_read
  file_table[fd].read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);//从文件偏移量开始读取len个字节
  file_table[fd].open_offset += len;//更新文件偏移量
  printf("read %u bytes from file:%s\n",len, file_table[fd].name);
  return len;*/
  //if(fd == -1) return events_read(buf, 0, len);
  Finfo *f = &file_table[fd];
  if (f->read == NULL) {//ramdisk_read进行读操作
    if(f->open_offset + len > f->size) {
      len = f->size - f->open_offset;
    }
    ramdisk_read(buf, f->disk_offset + f->open_offset, len);
    f->open_offset += len;
    return len;
  }
  return f->read(buf, f->disk_offset + f->open_offset, len);
}

size_t fs_write(int fd, const void *buf, size_t len) {
  /*if(fd == FD_STDOUT || fd == FD_STDERR){//标准输出和标准错误输出
    for(int i = 0; i < len; i++){
      putch(((char *)buf)[i]);
    }
    return len;
  }
  if(file_table[fd].open_offset + len > file_table[fd].size){
    len = file_table[fd].size - file_table[fd].open_offset;
  }
  file_table[fd].write = ramdisk_write;//文件写操作用ramdisk_write
  file_table[fd].write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);//从文件偏移量开始写入len个字节
  file_table[fd].open_offset += len;//更新文件偏移量
  return len;*/
  Finfo *f = &file_table[fd];
  if (f->write == NULL) {//ramdisk_write进行写操作
    if(f->open_offset + len > f->size) {
      len = f->size - f->open_offset;
    }
    ramdisk_write(buf, f->disk_offset + f->open_offset, len);
    f->open_offset += len;
    return len;
  }
  return f->write(buf, f->disk_offset + f->open_offset, len);
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  if(fd < 3) return 0;
  switch(whence){
    case SEEK_SET: file_table[fd].open_offset = offset; break;
    case SEEK_CUR: file_table[fd].open_offset += offset; break;
    case SEEK_END: file_table[fd].open_offset = file_table[fd].size + offset; break;
    default: panic("whence error");
  }
  return file_table[fd].open_offset;
}


int fs_close(int fd) {
  return 0;
}

size_t get_disk_offset(int fd){
  return file_table[fd].disk_offset;
}
void init_fs() {
  // TODO: initialize the size of /dev/fb
}
