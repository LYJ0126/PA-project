/*************************************************************************************** 
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University * 
* NEMU is licensed under Mulan PSL v2.  
* You can use this software according to the terms and conditions of the Mulan PSL v2.  
* You may obtain a copy of Mulan PSL v2 at: 
*          http://license.coscl.org.cn/MulanPSL2 * 
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  * 
* See the Mulan PSL v2 for more details.  
***************************************************************************************/
#include <isa.h>
#include <cpu/cpu.h> 
#include <readline/readline.h> 
#include <readline/history.h> 
#include "sdb.h" 
#include <memory/paddr.h>
static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
word_t expr(char *e, bool *success);
/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);//cpu_exec函数在nemu/src/cpu中
  return 0;
}

static int cmd_q(char *args) {
	nemu_state.state=NEMU_QUIT;//记得要修改nemu_state的状态，否则直接q推出main中执行最后一个函数后main会返回1而不是0
  return -1;//返回-1<0，经过mainloop中判断后会返回main函数
}

word_t paddr_read(paddr_t addr, int len);
void set_watchpoint(char* expression);
void delete_watchpoint(int no);
void sdb_watchpoint_display();
void save_regs(FILE *fp);
void save_mem(FILE *fp);
void load_regs(FILE *fp);
void load_mem(FILE *fp);
static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);
static int cmd_save(char *args);
static int cmd_load(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);//利用函数指针来表述cmd_table中每个命令所对应的函数。如命令"help"就对应cmp_help函数,这样能利用cmd_table[0]调用它。
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
	{"si", "让程序单步执行N条指令后暂停执行,当N没有给出时, 缺省为1,格式为si [N]",cmd_si},//单步执行
	{"info","打印程序状态,格式为info SUBCMD, info r:打印寄存器状态,info w:打印监视点信息",cmd_info},//打印程序状态
	{"x","扫描内存,格式为x N EXPR,求出表达式EXPR的值, 将结果作为起始内存地址, 以十六进制形式输出连续的N个4字节",cmd_x},//扫描内存
	{"p","表达式求值,格式为p EXPR,求出表达式EXPR的值",cmd_p},//表达式求值
	{"w","设置监视点,格式为w EXPR,求出表达式EXPR的值。当EXPR发生变化,暂停程序执行",cmd_w},//设置监视点
	{"d","格式为d N,删除编号为N的监视点",cmd_d},//删除监视点
  {"save","格式为save [path],将NEMU的当前状态保存到path指示的文件中",cmd_save},//保存快照
  {"load","格式为load [path],从path指示的文件中读入NEMU的状态",cmd_load},//读入快照
};

#define NR_CMD ARRLEN(cmd_table)//用宏定义NR_CMD为cmd_table的长度

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {//找到了help命令后的参数是cmd_table中第i个命令，则输出该命令名称以及描述
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);//找不到该参数
  }
  return 0;
}

static int cmd_si(char *args){
	uint64_t N=0;
	if(args==NULL) N=1;
	else {
		sscanf(args,"%lu",&N);
	}
	cpu_exec((uint64_t)N);//cpu_exec在nemu/src/cpu中
	return 0;
}
void sdb_set_batch_mode() {
  is_batch_mode = true;
}

static int cmd_info(char *args){
	char *arg = strtok(NULL," ");
	const char* r="r";
	const char* w="w";
	if(strcmp(arg,r)==0){
		isa_reg_display();//打印所有寄存器中的值,该函数在nemu/src/isa/$ISA/reg.c中
	}
	else if(strcmp(arg,w)==0){
		sdb_watchpoint_display();	
	}
	return 0;
}

static int cmd_x(char *args){
	uint32_t N;
	char* arg = strtok(args," ");
	if(arg==NULL){
		printf("缺少参数\n");
		return 0;
	}
	sscanf(arg,"%u",&N);
	char* arg2 = arg+strlen(arg)+1;
	if(arg2==NULL){
		printf("缺少参数\n");
		return 0;
	}
	bool* exprright = (bool*)malloc(sizeof(exprright));
	uint32_t addr = expr(arg2,exprright);
	if(*exprright==false){
		free(exprright);
		printf("表达式有误\n");
		return 0;
	}
	free(exprright);
	uint32_t tempaddr = 0;
	uint32_t value = 0;
	for(uint32_t i=0;i<N;i++){
		tempaddr=addr+4*i;
		value = paddr_read(tempaddr, 4);
		printf("0x%-8x\t 0x%-8x\t\n",tempaddr,value);
	}
	return 0;
}

static int cmd_p(char *args){
	bool *suc = (bool *)malloc(sizeof(bool));
	uint32_t tempvalue = expr(args, suc); 
	if(*suc == false) printf("表达式有误\n");
	else printf("%u\n",tempvalue);
	free(suc);
	return 0;
}

static int cmd_w(char *args){
	set_watchpoint(args);
	return 0;
}

static int cmd_d(char *args){
	if(args==NULL){
		printf("未输入要删除的监视点编号");
	}
	else delete_watchpoint(atoi(args));
	return 0;
}

static int cmd_save(char *args){
  if(args == NULL){
    printf("未输入文件名\n");
    return 0;
  }
  char *name = strtok(NULL, " ");//获取文件名
  char filename[100];
  strcpy(filename, "~/ics2023/nemu/src/monitor/snapshot/");
  strcat(filename, name);//~/ics2023/nemu/src/monitor/snapshot/name
  FILE *fp = fopen(filename, "w");
  if(fp == NULL){
    printf("文件打开失败\n");
    return 0;
  }
  save_regs(fp);
  save_mem(fp);
  fclose(fp);
  printf("保存至:%s\n",filename);
  return 0;
}

static int cmd_load(char *args){
  if(args == NULL){
    printf("未输入文件名\n");
    return 0;
  }
  char *name = strtok(NULL, " ");//获取文件名
  char filename[100];
  strcpy(filename, "~/ics2023/nemu/src/monitor/snapshot/");
  strcat(filename, name);//~/ics2023/nemu/src/monitor/snapshot/name
  FILE *fp = fopen(filename, "r");
  if(fp == NULL){
    printf("文件打开失败\n");
    return 0;
  }
  load_regs(fp);
  load_mem(fp);
  fclose(fp);
  printf("读取自:%s\n",filename);
  return 0;
}

void sdb_mainloop() {
  if (is_batch_mode) {//处于bath_mode状态,则调用cmd_c函数
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);//str_end指向str最后一个元素后一个“元素”(迭代器中的end)

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");//strtok分解字符串str,到空格处停止,分离出来的字符串作为命令,用于过滤空格
    if (cmd == NULL) { continue; }//空命令

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {//命令后面没有参数,这里设为NULL,后面调用cmd_talbe[i].handler(args)的时候传参为NULL
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif
	 //further parsing(进一步语法分析)
    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }//先执行函数cmd_table[i].handler(args),cmd_talbe[i]对应函数返回值小于0,返回main
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }//在cmd_table中找不到相应的命令
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
