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
#include <isa.h>
#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  word_t old_value;
  char *expr;

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
void free_wp(WP *wp) {
	if(wp == NULL) {
		return;
	}
	free(wp->expr);
// Add to free_
	wp->next = free_;
	free_ = wp;
}

bool del_wp(int NO) {
	WP* cur = head;
	WP* prev = NULL;
	while (cur != NULL) {
		if (cur->NO == NO) {
			break;
		}
	prev = cur;
	cur = cur->next;            //遍历列表
	}
	if (cur == NULL) {
		printf("Not found watchpoint [%d]\n", NO);
		return false;
	}
	if (prev == NULL) {
		head = cur->next;
	}
	else {
		prev->next = cur->next;
	}
	free_wp(cur);
	return true;
}

WP* new_wp() {
	if (free_ == NULL) {
		return NULL;
	}

	WP* wp = free_;
	free_ = free_->next;

	wp->next = head;
	head = wp;
	return wp;
}

bool check_wp() {
	bool success;
	WP* cur = head;
	while (cur != NULL) {
		word_t cur_val = expr(cur->expr, &success);
		if (cur_val != cur->old_value) {
			printf("\nWatchpoint [%d]: expr [%s]\n", cur->NO, cur->expr);
			printf("==> old value = 0x%016lx, new value = 0x%016lx\n", cur->old_value,cur_val);
			cur->old_value = cur_val;
			return true;
		}
	cur = cur->next;
	}
	return false;
}

int set_wp(char* e) {
	bool success;	
	word_t val = expr(e, &success);
	if (success == false) {
	return -1;
	}
	WP* wp = new_wp();
	wp->expr = strdup(e);     //指针
	wp->old_value = val;     
	return wp->NO;
}

void display_wp(){
	if(head==NULL)
		printf("watchpoint : None\n");
    else{
        printf("%-10s %-20s %-10s\n", "NO", "EXPR", "VALUE");
        for(WP *temp=head; temp!=NULL;temp=temp->next){
             printf("%-10d %-20s""hex: [ " FMT_WORD " ]" ", dec: [ %ld ]\n", temp->NO,temp->expr,temp->old_value,temp->old_value);
        }
    }
}

