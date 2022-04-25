#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void init_regex();
void cpu_exec(uint64_t);
/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
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
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
	char *str = strtok(NULL," ");
	int i=0;
	if(str==NULL){
		cpu_exec(1);
		return 0;
	}
	sscanf(str,"%d",&i);
	if(i==-1)
	{
		cpu_exec(-1);
	}
	if(i<-1)
	{
		printf("data error!\n");
		return 0;
	}
		cpu_exec(i);
		return 0;
}

static int cmd_info(char *args){
	char *str2 = strtok(NULL," ");
	if(strcmp(str2,"r")==0){
	for(int i=0;i<8;i++){
		printf("%s:\t%8x\t%d\n",regsl[i],cpu.gpr[i]._32,cpu.gpr[i]._32);
	}
	for(int i=0;i<8;i++){
	    printf("%s:\t%8x\t%d\n",regsw[i],cpu.gpr[i]._16,cpu.gpr[i]._16);
	}
    for(int i=0;i<8;i++){
     	printf("%s:\t%8x\t%d\n",regsb[i],cpu.gpr[i]._8[0],cpu.gpr[i]._8[0]);
	}
	printf("eip:  0x%-8x    %-8d\n", cpu.eip, cpu.eip);
}
else if(strcmp(args,"w")==0){
	list_watchpoint();
}
return 0;
}

static int cmd_p(char *args){
	init_regex();
    char *str=strtok(NULL,"");
    if(str==NULL)
    {
        printf("no input!");
    }
    else{
	//	printf("%s\n",str);
        bool q=true;
        uint32_t num=expr(str,&q);
        if(q){
            printf("answer=%d\n",num);
        }
       else{
           printf("bad input!");
       } 
    }
    return 0;
}

static int cmd_w(char *args){
	char *str=strtok(NULL,"");
	if(str){
		int num=set_watchpoint(str);
		if(num!=-1){
			printf("set watchpoint #%d\n",num);
		}
		else{printf("bad input!");}
	}
	return 0;
}

static int cmd_d(char *args){
	int num;
	sscanf(args,"%d",&num);
	if(!delete_watchpoint(num)){
		printf("Watchpoint #%d does not exist\n", num);
	}
	return 0;
}

static int cmd_x(char *args){
	char *str2=strtok(NULL," ");
	char *str3=strtok(NULL," ");
	vaddr_t addr;
	int leng;
	sscanf(str2,"%d",&leng);
	sscanf(str3,"%x",&addr);
	for(int i=0;i<leng;i++){
		printf("0x%x ",addr);
		printf("0x%08x ",vaddr_read(addr,4));
		int num=vaddr_read(addr,4);
		for(int j=0;j<4;j++){
			printf("%02x ",num%256);
			num=num/256;
		}
		addr+=4;
		printf("\n");
	}
	return 0;
}
static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si","Single step execution N instructions then pause",cmd_si},
  { "info","Print register",cmd_info},
  { "x","Scan memory",cmd_x},
  { "p","Expression evaluation",cmd_p},
  { "w","set watchpoint",cmd_w},
  { "d","delete watchpoint",cmd_d},
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

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

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
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

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
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
