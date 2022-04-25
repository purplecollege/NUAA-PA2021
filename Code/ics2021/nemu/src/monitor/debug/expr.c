#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include "monitor/expr.h"
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
	NUM,HEXNUM,REGNAME,ADD,LBRA,RBRA,MINUS,MULTIPLY,DIVIDE,TK_NEQ,AND,OR,NOT,DEREF,NEG
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", ADD},         // plus
  {"==", TK_EQ},         // equal
  {"\\b[0-9]+\\b", NUM},
  {"0[xX][0-9a-fA-F]+", HEXNUM},
  {"\\$[a-z]{2,3}", REGNAME},
  {"\\(",LBRA},
  {"\\)",RBRA},
  {"\\-",MINUS},
  {"\\*",MULTIPLY},
  {"\\/",DIVIDE},
  {"!=",TK_NEQ},
  {"&&",AND},
  {"\\|\\|",OR},
  {"!",NOT},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
//	printf("%s\n",e);
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
		tokens[nr_token].type=rules[i].token_type;
        switch (rules[i].token_type) {
			case TK_NOTYPE:
				break;
			case NUM:
			case HEXNUM:
			case ADD:
			case REGNAME:
			case LBRA:
			case RBRA:
			case MINUS:
			case MULTIPLY:
			case DIVIDE:
			case TK_EQ:
			case AND:
			case OR:
			case NOT:
			case TK_NEQ:
				strncpy(tokens[nr_token].str,substr_start,substr_len);
				nr_token++;
				break;
          default: TODO();
        }
		break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool judge_exp();
uint32_t eval(int p, int q);
uint32_t getnum(char str);
bool check_parentheses(int p, int q);
int find_dominant_operator(int p, int q);
uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  for(int i=0;i<nr_token;i++){
	//  printf("444");
	//  if(tokens[i].type=='-')
	 // {
	//	  printf("333");
	 //  }
	  if(tokens[i].type=='*'&&(i==0||(tokens[i-1].type=='+')||(tokens[i-1].type=='-')||(tokens[i-1].type=='(')||(tokens[i-1].type=='*'))){
						//	printf("222");
							tokens[i].type=DEREF;
				  }
	  if(tokens[i].type=='-'&&(i==0||(tokens[i-1].type=='+')||(tokens[i-1].type=='-')||(tokens[i-1].type=='(')||(tokens[i-1].type=='*'))){
							tokens[i].type=NEG;
						//	printf("222");
	  }
  }
  if (!judge_exp())
    *success = false;
  else
  return eval(0,nr_token-1);
  TODO();

  return 0;
}

uint32_t eval(int p, int q) {
  if (p > q){
	return 0;
  }
  else if (p == q){
//	  printf("%d",p);
    if (tokens[p].type == NUM) {
	//	printf("111");
      return atoi(tokens[p].str);
    }
    else if (tokens[p].type == REGNAME) {
//		vaddr_read(tokens[p].type,4);
      if (strcmp(tokens[p].str, "$eax") == 0) return cpu.eax;
      else if (strcmp(tokens[p].str, "$ebx") == 0)  return cpu.ebx;
      else if (strcmp(tokens[p].str, "$ecx") == 0)  return cpu.ecx;
      else if (strcmp(tokens[p].str, "$edx") == 0)  return cpu.edx;
      else if (strcmp(tokens[p].str, "$ebp") == 0)  return cpu.ebp;
      else if (strcmp(tokens[p].str, "$esp") == 0)  return cpu.esp;
      else if (strcmp(tokens[p].str, "$esi") == 0)  return cpu.esi;
      else if (strcmp(tokens[p].str, "$edi") == 0)  return cpu.edi;
      else if (strcmp(tokens[p].str, "$eip") == 0)  return cpu.eip;
    }
    else if (tokens[p].type == HEXNUM){
      int cnt=1, i, leng, sum = 0;
      leng = strlen(tokens[p].str);
      for (i = leng-1; i >= 0; i--) {
        sum = sum + cnt * getnum(tokens[p].str[i]);
        cnt *= 16;
      }
      return sum;
    }
  }
  else if (check_parentheses(p, q)){
    return eval(p + 1, q - 1);
  }
  else {
    int op = find_dominant_operator(p, q);
//	printf("%d\n",op);
    uint32_t val1 = eval(p, op - 1);
    uint32_t val2 = eval(op + 1, q);
    switch (tokens[op].type) {
      case ADD:
        return val1 + val2;
      case MINUS:
        return val1 - val2;
      case MULTIPLY:
        return val1 * val2;
      case DIVIDE:
        return val1 / val2;
      case AND:
        return val1 && val2;
      case OR:
        return val1 || val2;
      case TK_EQ:
        return val1 == val2;
      case TK_NEQ:
        return val1 != val2;
	  case DEREF:
	//	printf("111");
		return vaddr_read(val2,4);
	  case NEG:
		return -val2; 
      default:
		break;
    }
  }
  return 1;
}

uint32_t getnum(char str)
{
  if (str >= '0' && str <= '9') 
    return str - '0';
  else if (str >= 'a' && str <= 'f') 
    return str - 'a' + 10;
  else if (str >= 'A' && str <= 'F') 
    return str - 'A' + 10;
  return 0;
}
int youxian(int i);

bool check_parentheses(int p,int q){
	int flag=0;
	for(int i=p;i<=q;i++){
		if(tokens[i].type==LBRA){
			flag++;
		}
		if(tokens[i].type==RBRA){
			flag--;
		}
		if(flag==0&&i<q){
			return false;
		}
	}
	return true;
}

int find_dominant_operator(int p, int q) {
  int  leng,count=0;
  int op =10000, opp, flag = -1;
  for (int i = p; i <= q; i++){
    if (tokens[i].type==NUM||tokens[i].type==REGNAME||tokens[i].type==HEXNUM||strcmp(tokens[i].str," ")==0)
      continue;
    else if (tokens[i].type == LBRA) {
     leng = 0;
	 count++;
      for (int j = i + 1; j <= q; j++) {
        if (tokens[j].type == RBRA) {
          leng++;
		  count--;
		  if(count==0){
			  i+=leng;
			  break;
		  }
        }
        else
          leng++;
      }
    }
    else {
      opp = youxian(i);
      if (opp <= op) {
        flag = i;
        op = opp;
      }
    }
  }
  return flag;
}

int youxian(int i){
  if (tokens[i].type == ADD || tokens[i].type == MINUS) return 4;
  else if (tokens[i].type == MULTIPLY || tokens[i].type == DIVIDE) return 5;
  else if (tokens[i].type == TK_EQ||tokens[i].type ==TK_NEQ) return 3;
  else if (tokens[i].type == OR) return 1;
  else if (tokens[i].type == AND) return 2;
  else if (tokens[i].type == DEREF) return 6;
  return 100001;
}

bool judge_exp(){
  int i,flag;
  flag = 0;
  for (i = 0; i <= nr_token; i++) {
    if (tokens[i].type == LBRA)
      flag++;
    else if (tokens[i].type == RBRA)
      flag--;
    if (flag < 0)
      return false;
  }
  return true;
}
