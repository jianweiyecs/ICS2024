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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  int busy;
  char expr[100];
  uint32_t res;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    
    wp_pool[i].busy = 0;
    wp_pool[i].expr[0] = '\0';
    wp_pool[i].res = 0;
  }

  head = NULL;
  head->next = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
  if(!free_){
    printf("all wp is busy\n");
    assert(0);
  }
  WP* p = free_;
  free_ = free_->next;
  p->busy = 1;
  p->next = head->next;
  head = p;
  return p;
}

void free_wp(WP* wp){
  wp->busy = 0;

  if (wp == head)
  {
    head=head->next;
    wp->next = free_;
    free_ = wp;
  }else{
    WP* p = head;
    while (p->next != wp)
    {
      p=p->next;
    }
    p->next = wp->next;
    wp->next = free_;
    free_ = wp;
  }
  
}

WP* get_head(){
  return head;
}