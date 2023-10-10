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
	char exp[128];
	uint32_t old_value;

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

WP* new_wp(){
	if(free_==NULL){
		printf("没有空闲的监视点了\n");
		return NULL;
	}
	WP* temp = free_;
	free_ = free_->next;
	if(head==NULL){
		head = temp;
		head->next = NULL;
	}
	else{
		temp->next = head;
		head = temp;
	}
	return head;
}

void free_wp(WP* wp){
	//先检查wp是不是在链表head中
	WP* p = head;
	bool exist = false;
	while(p!=NULL){
		if(p->NO==wp->NO){
			exist = true;
			break;
		}
		p = p->next;
	}
	if(head==NULL) exist = false;
	if(!exist){
		printf("设置的监视点中并没有编号为#%d的\n",wp->NO);
		return;
	}
	if(p->NO==head->NO){
		head = head->next;
		p->next = free_;
		free_ = p;
	}
	else{
		WP* parent = head;
		while(parent->next!=p) parent = parent->next;
		parent->next=parent->next->next;
		p->next = free_;
		free_ = p;
	}
	printf("删除监视点#%d\n",wp->NO);
}

void set_watchpoint(char* expression){
	WP* p = new_wp();
	if(p==NULL) return;
	if(strlen(expression)>127){
		printf("表达式过长\n");
		return;
	}
	strcpy(p->exp,expression);
	bool* success = (bool*)malloc(sizeof(bool));
	uint32_t tmp = expr(expression,success);
	if(success){
		p->old_value = tmp;
		printf("创建监视点 #%d\n",p->NO);
		printf("expr: %s\n",p->exp);
		printf("old value: %u\n", tmp);
	}
	else{
		printf("创建监视点失败\n");
	}
	free(success);
	return;
}

void delete_watchpoint(int no)
{
		if(no<0||no>31){
			printf("没有编号为#%d的监视点\n",no);
			return;
		}
		for(int i=0;i<32;++i){
			if(wp_pool[i].NO == no){
				free_wp(&wp_pool[i]);
				break;
			}
		}
		return;
}

void sdb_watchpoint_display()
{
	WP* cur = head;
	if(cur==NULL){
		printf("没有监视点\n");
	}
	while(cur!=NULL){
		printf("number:%3d\t expr:%s\t old value:%u\t\n",cur->NO,cur->exp,cur->old_value);
		cur = cur->next;
	}
}

void scan_watchpoint()
{
	WP* cur = head;
	while(cur!=NULL){
		bool suc =false;
        uint32_t tempnum = expr(cur->exp,&suc);
        if(suc){
          if(tempnum!=cur->old_value){
            printf("监视点#%d的表达式的值已经改变\n",cur->NO);
            printf("expr:   %s\n",cur->exp);
            printf("old value:   %u\n",cur->old_value);
            printf("new value:   %u\n",tempnum);
            cur->old_value = tempnum;//更新
            nemu_state.state = NEMU_STOP;//暂停
          }
        }
        else{
          printf("expression error!\n");
        }
				cur = cur->next;
	}
}
