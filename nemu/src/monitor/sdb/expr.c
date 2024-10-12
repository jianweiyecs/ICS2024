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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_NUM,
  TK_ADD,
  TK_SUB,
  TK_MUL,
  TK_DIV,
  TK_ZUO,
  TK_YOU,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */


  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_ADD},         // plus

  
  {"\\-", TK_SUB}, //sub
  {"\\*", TK_MUL}, //mul
  {"\\/", TK_DIV}, //div
  {"\\(", TK_ZUO},
  {"\\)", TK_YOU},
  {"[0-9]*", TK_NUM},
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
  // printf("Inter make_token\n");
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
          case TK_ADD: 
            tokens[nr_token].type = TK_ADD;
            nr_token++;
            break;
          case TK_SUB: 
            tokens[nr_token].type = TK_SUB;
            nr_token++;
            break;
          case TK_MUL: 
            tokens[nr_token].type = TK_MUL;
            nr_token++;
            break;
          case TK_DIV: 
            tokens[nr_token].type = TK_DIV;
            nr_token++;
            break;
          case TK_ZUO: 
            tokens[nr_token].type = TK_ZUO;
            nr_token++;
            break;
          case TK_YOU: 
            tokens[nr_token].type = TK_YOU;
            nr_token++;
            break;
          case TK_NUM: {
            tokens[nr_token].type = TK_NUM;
            if(substr_len >= 32){
              printf("buffer overflow in INT, buffer is 32bit, shoulde give 31bit, last bit is \\0\n");
              assert(0);
            }
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            nr_token++;
            break;
          }
          case TK_NOTYPE:break;
          default: 
            printf("No this rule\n");
            break;
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


static int SUCCESS = 1;


static bool check_parentheses(int l, int r){
  if (tokens[l].type == '(' && tokens[r].type == ')'){
    return true;
  }
  return false;
}

static bool check_in_parentheses(int index){
  int i;
  for(i = index; i>=0; i--){
    if(tokens[i].type == '('){
      return true;
    }
  }
  for(i = index;i < nr_token;i++){
    if(tokens[i].type == ')'){
      return true;
    }
  }
  return false;
}
static int find_op(Token* p){
  int op_type = -1;
  int op = 0;
  int i;
  for(i = 0;i < nr_token; i++){
    switch (tokens[i].type)
    {
    case TK_ADD:{
      if (op_type <= 1 && !check_in_parentheses(op)){
        op_type = 1;
        op = i;
      }
      break;
    }
    case TK_SUB:{
      if (op_type <= 1 && !check_in_parentheses(op)){
        op_type = 1;
        op = i;
      }
      break;
    }
    case TK_MUL:{
      if (op_type <= 0 && !check_in_parentheses(op)){
        op_type = 0;
        op = i;
      }
      break;
    }
    case TK_DIV:{
      if (op_type <= 0 && !check_in_parentheses(op)){
        op_type = 0;
        op = i;
      }
      break;
    }
    default:
      break;
    }
  }
  return op;
}

static int eval(int l,int r){
  if(l > r){
    // printf("The expression is illegal, please re-enter it\n");
    SUCCESS = 0;
    return 0;
  }else if(l == r){
    return atoi(tokens[l].str);
  }else if(check_parentheses(l, r)){
    eval(l + 1, r - 1);
  }else{
    int op = find_op(tokens);
    int val1 = eval(l, op - 1);
    int val2 = eval(op + 1, r);
    switch (tokens[op].type)
    {
    case TK_ADD:{
      return val1 + val2;
    }
    case TK_SUB:{
      return val1 - val2;
    }
    case TK_MUL:{
      return val1*val2;
    }
    case TK_DIV:{
      if(val2 == 0){
        SUCCESS = 0;
        printf("The expression is illegal, please re-enter it, find //0\n");
        return 0;
      }else{
        return val1/val2;
      }
    }
    default:
      break;
    }
  }
  return 0;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  //TODO();


  SUCCESS = 1;
  int res = eval(0,nr_token - 1);
  if(SUCCESS){
    printf("%s = %d\n", e, res);
    return res;
  }else{
    return 0;
  }
}
