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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define BUFMAX 550
// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";


static uint32_t choose(int n){
  return rand() % n;
}

static void gen_num(){

  char digit[32];
  digit[0] = '\0';
  uint32_t res = rand() % 10 + 1;
  sprintf(digit,"%u",res);
  strcpy(buf + strlen(buf),digit);
}

static void gen(char c){
  sprintf(buf + strlen(buf), "%c", c);
}

static void gen_rand_op(){

  uint32_t res = rand() % 4;
  char c;
  switch (res)
  {
  case 0:
    c = '+';
    sprintf(buf + strlen(buf), "%c", c);
    break;
  case 1:
    c = '-';
    sprintf(buf + strlen(buf), "%c", c);
    break;
  case 2:
    c = '*';
    sprintf(buf + strlen(buf), "%c", c);
    break;
  case 3:
    c = '/';
    sprintf(buf + strlen(buf), "%c", c);
    break;
  default:
    break;
  }
} 

static void gen_rand_expr() {
  switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
}

static int legal(int i){
  if(buf[i] == '/' && (buf[i+1]==')' || buf[i+1] == '(')){
    return 0;
  }
  if(buf[i + 1] == '/' && (buf[i]==')' || buf[i] == '(')){
    return 0;
  }
  return 1;
}
int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf[0] = '\0';
    code_buf[0] = '\0';
    gen_rand_expr();
    sprintf(code_buf, code_format, buf);
    int j;
    if(strlen(buf) > BUFMAX){
      continue;
    }
    for(j = 0;j < strlen(buf) - 1; j++){
      if(!legal(j)){
        continue;
      }
    }
    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
