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
extern size_t get_disk_offset(int fd);
//extern size_t fs_write(int fd, const void *buf, size_t len);
//size_t get_ramdisk_size();
static uintptr_t loader(PCB *pcb, const char *filename) {
  //TODO();
  
  //从文件中读取ELF头
  Elf_Ehdr elf;
  //printf("offset:0,len:%d,ramdisk_size:%x\n",sizeof(Elf_Ehdr),get_ramdisk_size());
  //ramdisk_read(&elf, 0, sizeof(Elf_Ehdr));
  int fd = fs_open(filename);
  fs_read(fd, &elf, sizeof(Elf_Ehdr));
  size_t disk_offset = get_disk_offset(fd);
  //printf("elf.e_phnum:%d\n",elf.e_phnum);
  assert(elf.e_ident[0] == 0x7f && elf.e_ident[1] == 'E' && elf.e_ident[2] == 'L' && elf.e_ident[3] == 'F');//0x7fELF
  // 读取program header，并加载到内存中
  for (int i = 0; i < elf.e_phnum; i++) {
    Elf_Phdr phdr;
    ramdisk_read(&phdr, disk_offset+elf.e_phoff + i * elf.e_phentsize, sizeof(Elf_Phdr));
    if (phdr.p_type == PT_LOAD) {
      ramdisk_read((void *)phdr.p_vaddr, disk_offset + phdr.p_offset, phdr.p_memsz);
      //printf("read phdr.p_vaddr:%x,phdr.p_offset:%x,phdr.p_memsz:%x\n",phdr.p_vaddr,phdr.p_offset,phdr.p_memsz);
      memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
    }
  }
  //跳转到程序入口执行
  printf("elf.e_entry:%x\n",elf.e_entry);
  return elf.e_entry;
  
  /*
  printf("filename:%s\n",filename);
  int fd = fs_open(filename);
  printf("fd:%d\n",fd);
  Elf_Ehdr elf;
  fs_read(fd, &elf, sizeof(Elf_Ehdr));
  printf("elf.e_phnum:%d\n",elf.e_phnum);
  assert(elf.e_ident[0] == 0x7f && elf.e_ident[1] == 'E' && elf.e_ident[2] == 'L' && elf.e_ident[3] == 'F');//0x7fELF
  for (int i = 0; i < elf.e_phnum; i++) {
    Elf_Phdr phdr;
    fs_read(fd, &phdr, sizeof(Elf_Phdr));
    if (phdr.p_type == PT_LOAD) {
      fs_read(fd, (void *)phdr.p_vaddr, phdr.p_memsz);
      printf("read phdr.p_vaddr:%x,phdr.p_offset:%x,phdr.p_memsz:%x\n",phdr.p_vaddr,phdr.p_offset,phdr.p_memsz);
      memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
    }
  }
  printf("elf.e_entry:%x\n",elf.e_entry);
  return elf.e_entry;
  */
}

void naive_uload(PCB *pcb, const char *filename) {
  //printf("start naive_uload\n");
  uintptr_t entry = loader(pcb, filename);
  //printf("end naive_uload\n");
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  printf("context_uload\n");
  uintptr_t entry = loader(pcb, filename);
  Area ustack;
  ustack.start = pcb->stack;
  ustack.end = pcb->stack + STACK_SIZE;
  printf("ustack.start:%x,ustack.end:%x\n",ustack.start,ustack.end);
  //计算argc
  int tempargc = 0;
  if(argv == NULL) tempargc = 0;
  else{
    while(argv[tempargc] != NULL) tempargc++;
  }
  int argc = tempargc;//参数个数
  //printf("argc:%d\n",argc);
  //计算envp参数个数
  int numenvp = 0;
  if(envp == NULL) numenvp = 0;
  else{
    while(envp[numenvp] != NULL) numenvp++;
  }
  //printf("numenvp:%d\n",numenvp);
  /*
  char *ustack_sp = heap.end;
  //(uintptr_t*)ustack_sp = (uintptr_t*)heap.end;
  char *args[argc];//记录每个参数的地址
  char *envp_sp[numenvp];//记录每个环境变量的地址
  for(int i = 0; i < argc; i++){
    ustack_sp -= (strlen(argv[i]) + 1);
    strcpy(ustack_sp, argv[i]);
    args[i] = ustack_sp;
  }
  for(int i = 0; i < numenvp; i++){
    ustack_sp -= (strlen(envp[i]) + 1);
    strcpy(ustack_sp, envp[i]);
    envp_sp[i] = ustack_sp;
  }
  char **sp = (char **)ustack_sp;
  sp--;
  *sp = NULL;
  for(int i = numenvp - 1; i >= 0; i--){
    sp--;
    *sp = envp_sp[i];
  }
  sp--;
  *sp = NULL;
  for(int i = argc - 1; i >= 0; i--){
    sp--;
    *sp = args[i];
  }
  sp--;
  *((int *)sp) = argc;
  Context *c = ucontext(NULL, ustack, (void *)entry);
  printf("创建用户进程的上下文\n");
  pcb->cp = c;
  pcb->cp->GPRx = (uintptr_t)sp;
  */
  uintptr_t *ustack_sp = (uintptr_t *)heap.end;//ustack.end;
  char *args[argc];//记录每个参数的地址
  char *envp_sp[numenvp];//记录每个环境变量的地址
  for(int i=0;i<argc;i++){
    uint32_t len = strlen(argv[i]) + 1;
    ustack_sp -= len;
    strncpy((char *)ustack_sp, argv[i], len);
    args[i] = (char *)ustack_sp;
  }
  for(int i=0;i<numenvp;i++){
    uint32_t len = strlen(envp[i]) + 1;
    ustack_sp -= len;
    strncpy((char *)ustack_sp, envp[i], len);
    envp_sp[i] = (char *)ustack_sp;
  }
  ustack_sp = ustack_sp - 1 - numenvp - 1 - argc - 1;
  ustack_sp[0] = argc;
  //uintptr_t *sp = ustack_sp;
  for(int i=0;i<argc;++i){
    ustack_sp[i+1] = (uintptr_t)args[i];
  }
  ustack_sp[argc+1] = 0;//argv[argc] = NULL
  for(int i=0;i<numenvp;++i){
    ustack_sp[argc+2+i] = (uintptr_t)envp_sp[i];
  }
  ustack_sp[argc+numenvp+2] = 0;//envp[numenvp] = NULL
  Context *c = ucontext(NULL, ustack, (void *)entry);
  pcb->cp = c;
  pcb->cp->GPRx = (uintptr_t)ustack_sp;
}


