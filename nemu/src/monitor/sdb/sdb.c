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
#include <memory/vaddr.h>
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
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
	nemu_state.state=NEMU_QUIT;   //解决退出时的warn
	return -1;
}
static int cmd_si(char *args) {
	int cpu_step = 1;	
	if (args != NULL) {
		cpu_step = atoi(args);
		}
	cpu_exec(cpu_step);	
	return 0;
}
static int cmd_info(char *args){
	if(strcmp(args,"r")==0){
	isa_reg_display();}
	if(strcmp(args,"w")==0){
	display_wp();}
	return 0;
}

static int cmd_x(char *args){
	char* number = strtok(args," ");
	if(number == NULL){
		printf("Error cmd_x");
		return 0;
	}                           //查找第二个空格 “x number input_addr”
	int N=atoi(number);
	bool success= true;
	vaddr_t base_addr = expr(number + strlen(number) + 1, &success);
	if (!success) {
		printf("Bad expr!\n");
		return 0;
	}
	int word_size = sizeof(word_t);
	for (int i = 0; i < N; ++i) {
		word_t data = vaddr_read(base_addr + i * word_size, word_size);
		printf(FMT_WORD ": ", (base_addr + i * word_size));
		//for (int j = 0; j < word_size; ++j) {
			//printf("0x%02x ", (uint8_t) (data&0xFF));
			//data >>= 8;
		//}
		printf("%016lx",data);
		printf("\n");
	}
	return 0;
	
}

static int cmd_p(char *args) {
	if (args == NULL) {
		return 0;
	}
	bool success = false;
	word_t val = expr(args, &success);
	if (!success) return 0;
	printf("value== " "hex: [ " FMT_WORD " ]" ", dec: [ %ld ]\n", val, val);
	return 0;
}

static int cmd_w(char *args) {
	if (args == NULL) {
		return 0;
	}
	int NO = set_wp(args);
	if (NO == -1) {
		printf("Bad expr!\n");
		return 0;
	}
	printf("Set watchpoint [%d]\n", NO);
	return 0;
}

static int cmd_d(char *args) {
	int NO;	
	if (args == NULL) {
		return 0;
		}
	NO = atoi(args);	
	if(del_wp(NO))
	printf("Successfully Delete %d watchpoint",NO);
	return 0;
}


static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);       //函数指针
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Single instruction", cmd_si },
  { "info", "Print register information  r for register m for memory ", cmd_info },
  { "x","Scan Memory", cmd_x},
  { "p","Calculate expression",cmd_p},
  { "w","Set new watch point",cmd_w},
  { "d","delete watchpoint",cmd_d},
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
