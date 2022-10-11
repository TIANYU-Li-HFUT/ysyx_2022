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
//#include <reg.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,TK_NEQ,TK_NUM,TK_REG,TK_PLUS,TK_SUB,TK_MUL,
  TK_DIV,TK_AND,TK_OR,TK_XOR,TK_NOT,TK_DREF,TK_LBRA,TK_RBRA,TK_NEG,

  /* TODO: Add more token types */

};  


static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  
  {"0[xX][0-9a-fA-F]+", TK_NUM}, // hex number
  {"[0-9]+", TK_NUM}, // dec number
  {"\\$[0-9a-zA-Z]+", TK_REG}, 

  {"\\+", TK_PLUS},         
  {"-", TK_SUB},
  {"\\*", TK_MUL}, 
  {"/", TK_DIV}, 

  {"==", TK_EQ},        
  {"!=", TK_NEQ},       
  {"&&", TK_AND}, 
  {"\\|\\|", TK_OR}, 
  {"\\^", TK_XOR},  
  {"!", TK_NOT},  
  
  {"!", TK_DREF}, 
  
  {"\\(", TK_LBRA}, // left bracket
  {"\\)", TK_RBRA}, // right bracket
  
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

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

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
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

        switch (rules[i].token_type) {
         case TK_NOTYPE:
           break;
         case TK_NUM:
         case TK_REG:
           strncpy(tokens[nr_token].str, substr_start, substr_len);
        default:
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;

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
int set_priority(int calculate_token){    //设定计算优先级 参考Verilog
	int priority=-1;
	switch(calculate_token){
		case TK_NOT:
			priority=0;
			break;
		case TK_MUL:
			priority=1;
			break;
		case TK_DIV:
			priority=1;
			break;
		case TK_PLUS:
			priority=2;
			break;
		case TK_SUB:
			priority=2;
			break;		
		case TK_EQ:
			priority=3;
			break;
		case TK_NEQ:
			priority=3;
			break;
		case TK_AND:
			priority=4;
			break;
		case TK_XOR:
			priority=5;
			break;
		case TK_OR:
			priority=6;
			break;	
	}
	return(priority);
}
int is_operator(int symbol) {
	int priority = set_priority(symbol);
	if (priority >= 0) return 1;
	else return 0;
}
int compare_priority(int op1, int op2) {
	int p1 = set_priority(op1);
	int p2 = set_priority(op2);
	int val= p1 - p2;
return val;
}

int dominant_operator(int p, int q) {
	int counter = 0, dominant = -1;
	for (int i = p; i <= q; i++) {
		if (counter == 0 && is_operator(tokens[i].type)) {
			if ((dominant == -1) ||(compare_priority(tokens[i].type,tokens[dominant].type) <= 0)){
				dominant = i;
			}
		}
		else if (tokens[i].type == TK_LBRA) counter += 1;
		else if (tokens[i].type == TK_RBRA) counter -= 1;
	}
return dominant;
}

int check_parentheses(int p, int q) {
	bool surrounded = 0;
	if (tokens[p].type == TK_LBRA && tokens[q].type == TK_RBRA) {
		p += 1;
		q -= 1;
		surrounded = 1;
	}
	int counter = 0;
	for (int i = p; i <= q; ++i) {
		switch (tokens[i].type) {
			case TK_LBRA: counter += 1; break;
			case TK_RBRA: counter -= 1; break;
			default: break;
		}
	}
	if (counter != 0 ){
		return -1;
	}
	else if (surrounded ) {
		return 1;
	} 
	else {
		return 0;
	}
}
word_t eval(int p, int q, bool *success) {
	if (p > q) {
		*success = false;
		return 0;
	}
/* Single token */
	else if (p == q) {
		int type = tokens[p].type;
		if (type == TK_NUM || type == TK_NUM) {
			*success = true;
			return strtoul(tokens[p].str, NULL, 0);
		}
		if (type == TK_REG) {
			word_t val = isa_reg_str2val(tokens[p].str, success);
			
			printf("get %s value \n",tokens[p].str);
			if (*success) return val;
			return 0;
		}
		*success = false;
		return 0;
	}
	int retval = check_parentheses(p, q);
	if (retval == -1) {
		*success = false;
		return 0;
	}
	if (retval == 1) {
		return eval(p+1, q-1, success);
	}
	word_t val = 0, val1 = 0, val2 = 0;
	int op_dominant = dominant_operator(p, q);
	if (op_dominant < 0) return 0;
	int type = tokens[op_dominant].type;
	if (type == TK_NOT || type == TK_NEG || type == TK_DREF) {
		val = eval(p+1, q, success);
		if (*success == false) return 0;
		switch (type) {
			case TK_NOT: return !val;
			case TK_NEG: return -val;
			case TK_DREF: return vaddr_read(val, sizeof(word_t));
			default: assert(0);
		}
	}
	val1 = eval(p, op_dominant - 1, success);
	if (*success == false) return 0;
	val2 = eval(op_dominant + 1, q, success);
	if (*success == false) return 0;
	switch (tokens[op_dominant].type) {
		case TK_PLUS: return val = val1 + val2;	
		case TK_SUB: return val = val1 - val2;
		case TK_MUL: return val = val1 * val2;
		case TK_DIV:
			if (val2 == 0) {
				printf("Divide 0 error!\n");
				*success = false;
				return 0;
			}
			return val = val1 / val2;
		case TK_AND: return val = val1 && val2;
		case TK_OR: return val = val1 || val2;
		case TK_XOR: return val = val1 ^ val2;
		case TK_EQ: return val = val1 == val2;
		case TK_NEQ: return val = val1 != val2;
		default:
			printf("Unknown token type: %d\n", tokens[op_dominant].type);
			*success = false;
	return 0;
	}
}
word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
 	for (int i = 0; i < nr_token; i++) {
		if (tokens[i].type == '-') {
			if (i == 0) {
				tokens[i].type = TK_NEG;
				continue;
			}
			int prev_type = tokens[i-1].type;
			if (prev_type != ')' && prev_type != TK_NUM && prev_type != TK_REG) {
				tokens[i].type = TK_NEG;
			}
		}
		if (tokens[i].type == '*') {
			if (i == 0) {
				tokens[i].type = TK_DREF;
				continue;
			}
			int prev_type = tokens[i-1].type;
			if (prev_type != ')' && prev_type != TK_NUM && prev_type != TK_REG) {
				tokens[i].type = TK_DREF;
			}
		}
	}

  return eval(0, nr_token - 1, success);;
}
