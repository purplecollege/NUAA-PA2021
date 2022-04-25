#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32
#include<stdlib.h>
static WP wp_pool[NR_WP];
static WP *head, *free_;

WP* new_wp();
void free_wp(WP *wp);

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
	if(free_ == NULL){
		assert(0);
	}
	WP *node = free_;
	free_=free_->next;
	node->next=NULL;
	return node;
}

void free_wp(WP *wp){
	assert(wp >= wp_pool && wp < wp_pool + NR_WP);
	free(wp->exp);
	wp->next = free_;
	free_ =wp;
}

int set_watchpoint(char *e) {
	uint32_t val;
	bool success;
	val = expr(e, &success);
	if(!success) return -1;
	WP *p = new_wp();
	p->exp=strdup(e);
	p->old_val = val;
	p->next = head;
	head = p;
	return p->NO;
}

bool delete_watchpoint(int NO) {
	WP *p, *prev = NULL;
	for(p = head; p != NULL; prev = p, p = p->next) {
		if(p->NO == NO) { break; }
	}
	if(p == NULL) { return false; }
	if(prev == NULL) { head = p->next; }
	else { prev->next = p->next; }
	free_wp(p);
	return true;
}

void list_watchpoint() {
	if(head == NULL) {
		printf("No watchpoints\n");
		return;
	}

	printf("%8s\t%8s\t%8s\n", "NO", "Address", "Enable");
	WP *p;
	for(p = head; p != NULL; p = p->next) {
		printf("%8d\t%s\t%#08x\n", p->NO, p->exp, p->old_val);
	}
}

WP* scan_watchpoint(){
	WP *p;
	for(p = head; p != NULL; p = p->next) {
		bool success;
		p->new_val = expr(p->exp, &success);
		if(p->old_val != p->new_val) {
			return p;
		}
	}
	return NULL;
}
