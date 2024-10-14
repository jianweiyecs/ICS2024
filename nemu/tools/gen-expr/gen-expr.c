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

static uint32_t choose(){
  srand(time(NULL));
  return rand() % 2;
}

static void gen_num(){
  srand(time(NULL));
  uint32_t res = rand() % UINT32_MAX;
  snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%u", res);
  if (res == 0 && buf[strlen(buf) - 2] == '/'){
    buf[strlen(buf) - 1] = '1';
  }
}

static void gen(char c){
  snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%c", c);
}

static void gen_rand_op(){
  srand(time(NULL));
  uint32_t res = rand() % 4;
  char c;
  switch (res)
  {
  case 0:
    c = '+';
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%c", c);
    break;
  case 1:
    c = '-';
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%c", c);
    break;
  case 2:
    c = '*';
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%c", c);
    break;
  case 3:
    c = '/';
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%c", c);
    break;
  default:
    break;
  }
} 

static void gen_rand_expr() {
  switch (choose()) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
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
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

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
