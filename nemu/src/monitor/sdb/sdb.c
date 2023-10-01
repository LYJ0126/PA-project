/*************************************************************************************** * Copyright (c) 2014-2022 Zihao Yu, Nanjing University * * NEMU is licensed under Mulan PSL v2.  * You can use this software according to the terms and conditions of the Mulan PSL v2.  * You may obtain a copy of Mulan PSL v2 at: *          http://license.coscl.org.cn/MulanPSL2 * * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

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
  return -1;//返回-1<0，经过mainloop中判断后会返回main函数
	nemu_state.state=NEMU_QUIT;//记得要修改nemu_state的状态，否则直接q推出main中执行最后一个函数后main会返回1而不是0
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);//利用函数指针来表述cmd_table中每个命令所对应的函数。如命令"help"就对应cmp_help函数,这样能利用cmd_table[0]调用它。
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */

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

void sdb_set_batch_mode() {
  is_batch_mode = true;
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
    if (cmd == NULL) { continue; }

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
        if (cmd_table[i].handler(args) < 0) { return; }//cmd_talbe[i]对应函数返回值小于0,返回main
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
