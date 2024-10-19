/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

#include <memory/paddr.h>
NEMUState nemu_state;

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  int busy;
  char expr[100];
  uint32_t res;
} WP;
WP* get_head();
WP* new_wp();
void free_wp(WP* wp);
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
  cpu_exec(-1);
  return 0;
}

/*PA1的小练习，发现当没有执行c而是直接q退出时，会有error，由于没有执行指令，NEMU的state没有改变，依旧会是NEMU_Stop，因此想到在这里（当执行q时候，一定必须将NEMU的state设置为END*/
static int cmd_q(char *args) {
  nemu_state.state = NEMU_END;
  return -1;
}

static int cmd_si(char* args){
  int n;
  sscanf(args,"%d",&n);
  int insructions = n > 1 ? n : 1;
  int i;
  for(i = 1;i<=insructions;i++){
    cpu_exec(1);
  }
  return 0;
}

static int cmd_info(char* args){
  char c;
  sscanf(args,"%c",&c);
  if(c == 'r' || c == 'R'){
    isa_reg_display();
  }else{
    printf("Unknow command\n");
  }

  if(c == 'w' || c == 'W'){
    WP* p = get_head();
    printf("WP Info :\n");
    while(p){
      printf("Num%d, expr:%s\n", p->NO, p->expr);
      p=p->next;
    }
  }
  return 0;
}

static int cmd_p(char* args){
  char e[100];
  // sscanf(args,"%s",e);
  strcpy(e,args);
  bool success = true;
  int res = expr(e, &success);
  if(success){
    return res;
  }else{
    return 0;
  }
}

static int cmd_x(char* args){
  uint32_t address;
  int n;
  sscanf(args,"%d %x",&n,&address);
  int i;
  for(i = 0;i < n;i++){
    printf("address: %x  memory: %08x\n",address,paddr_read(address,4));
    address = address + 4;
  }
  return 0;
}

static int cmd_w(char* args){
  char exprs[100];
  strcpy(exprs, args);

  WP* p = new_wp();
  strcpy(p->expr,exprs);

  bool flag = true;
  uint32_t res = expr(exprs, &flag);
  if(flag){
    p->res = res;
  }
  return 0;
}

static int cmd_d(char *args){
  int num;
  sscanf(args,"%d", &num);
  
  WP* p = get_head();
  bool delete = false;
  while (!p)
  {
    if(p->NO == num){
      delete = true;
      free_wp(p);
      break;
    }
  }
  if(!delete){
    printf("wp num:%d not in busy_list\n", num);
  }
  return 0;
}
static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Execute n instructions",cmd_si},
  { "info", "Print register values",cmd_info},
  { "p", "Expression evaluation",cmd_p},
  { "x","Scan Memory",cmd_x},
  { "w", "When the value of expression EXPR changes, the program execution is paused.", cmd_w},
  { "d", "delete wp", cmd_d},
};

#define NR_CMD ARRLEN(cmd_table)

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
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
