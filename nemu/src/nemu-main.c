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

#include <common.h>
// word_t expr(char *e, bool *success);
void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /* Start engine. */
  engine_start();

  /* Test expr */

  // FILE *fp = fopen("/home/yejianwei/projects/ics2024/nemu/tools/gen-expr/input", "w");
  // assert(fp != NULL);
  // char line[600];
  // char exprs[560];
  // int error = 0;
  // while (fgets(line, sizeof(line), fp) != NULL) {
  //     uint32_t res;
  //     sscanf(line, "%u %s" ,&res, exprs);
  //     bool expr_flag = true;
  //     uint32_t res2 = (uint32_t)expr(exprs, &expr_flag);
  //     if(res != res2){
  //       error++;
  //       printf("res = %u res2 = %u expr = %s\n", res, res2, exprs);
  //     }
  // }
  // printf("error: %d\n", error);
  return is_exit_status_bad();
}
