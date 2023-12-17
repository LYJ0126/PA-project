#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif
extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
//extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);
extern int fs_open(const char *pathname);
extern size_t fs_read(int fd, void *buf, size_t len);
//extern size_t fs_write(int fd, const void *buf, size_t len);
//size_t get_ramdisk_size();
static uintptr_t loader(PCB *pcb, const char *filename) {
  //TODO();
  /*
  //从文件中读取ELF头
  Elf_Ehdr elf;
  //printf("offset:0,len:%d,ramdisk_size:%x\n",sizeof(Elf_Ehdr),get_ramdisk_size());
  //ramdisk_read(&elf, 0, sizeof(Elf_Ehdr));
  fs_read(fs_open(filename), &elf, sizeof(Elf_Ehdr));
  assert(elf.e_ident[0] == 0x7f && elf.e_ident[1] == 'E' && elf.e_ident[2] == 'L' && elf.e_ident[3] == 'F');//0x7fELF
  // 读取program header，并加载到内存中
  for (int i = 0; i < elf.e_phnum; i++) {
    Elf_Phdr phdr;
    ramdisk_read(&phdr, elf.e_phoff + i * elf.e_phentsize, sizeof(Elf_Phdr));
    if (phdr.p_type == PT_LOAD) {
      ramdisk_read((void *)phdr.p_vaddr, phdr.p_offset, phdr.p_memsz);
      memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
    }
  }
  //跳转到程序入口执行
  return elf.e_entry;
  */
  
  printf("filename:%s\n",filename);
  int fd = fs_open(filename);
  printf("fd:%d\n",fd);
  Elf_Ehdr elf;
  fs_read(fd, &elf, sizeof(Elf_Ehdr));
  assert(elf.e_ident[0] == 0x7f && elf.e_ident[1] == 'E' && elf.e_ident[2] == 'L' && elf.e_ident[3] == 'F');//0x7fELF
  for (int i = 0; i < elf.e_phnum; i++) {
    Elf_Phdr phdr;
    fs_read(fd, &phdr, sizeof(Elf_Phdr));
    if (phdr.p_type == PT_LOAD) {
      fs_read(fd, (void *)phdr.p_vaddr, phdr.p_memsz);
      memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
    }
  }
  return elf.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  printf("start naive_uload\n");
  uintptr_t entry = loader(pcb, filename);
  printf("end naive_uload\n");
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

