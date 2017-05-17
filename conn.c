#include <stdio.h>
#include <stdlib.h>
#include "conn.h"
#include "dls.h"

conn_t *head_conn = NULL;
conn_t *tail_conn = NULL;
unsigned int conn_num = 0;

void insert_conn(conn_t *ptr) {
	conn_num++;

	ptr->prev=NULL;
	ptr->next=NULL;
	ptr->ht_lock_node = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
	ptr->head=NULL;
	ptr->tail=NULL;

	if(tail_conn) {
		ptr->prev = tail_conn;
		tail_conn->next=ptr;
		tail_conn=ptr;
	} else {
		head_conn=tail_conn=ptr;
	}
}

void remove_conn(conn_t *ptr) {
	conn_num--;

	if(head_conn == tail_conn) {
		head_conn = tail_conn = NULL;
	} else if(ptr->next == NULL) {
		ptr->prev->next = NULL;
		tail_conn = ptr->prev;
	} else if(ptr->prev == NULL) {
		ptr->next->prev = NULL;
		head_conn = ptr->next;
	} else {
		ptr->next->prev = ptr->prev;
		ptr->prev->next = ptr->next;
	}

	register conn_node_t *p=ptr->tail,*tmp;
	register lock_t *lock;
	while(p) {
		lock=p->node->lock;

		if(p->node==lock->head && lock->head->next) {
			send_status(lock->head->next->conn->sockfd, true);
		}

		dprintf("%s: %s(%d)\n", __func__, lock->key, lock->head==p->node?1:0);

		remove_lock_node(p->node);

		tmp=p;
		p=p->prev;
		free(tmp);
	}

	shutdown(ptr->sockfd,2);
	close(ptr->sockfd);

	event_del(&ptr->event);
	g_hash_table_destroy(ptr->ht_lock_node);

	free(ptr);
}

void clean_conn() {
	register conn_t *ptr=head_conn,*tmp;
	register conn_node_t *p,*tmp2;
	register lock_t *lock;
	while(ptr) {
		shutdown(ptr->sockfd,2);
		close(ptr->sockfd);
		
		p=ptr->tail;
		while(p) {
			lock=p->node->lock;

			if(p->node==lock->head && lock->head->next) {
				send_status(lock->head->next->conn->sockfd, true);
			}

			dprintf("%s: %s(%d)\n", __func__, lock->key, lock->head==p->node?1:0);

			remove_lock_node(p->node);

			tmp2=p;
			p=p->prev;
			free(tmp2);
		}

		event_del(&ptr->event);
		g_hash_table_destroy(ptr->ht_lock_node);

		tmp=ptr;
		ptr=ptr->next;

		free(tmp);
	}

	conn_num = 0;
}

void insert_lock_node(lock_node_t *ptr) {
	ptr->lock->node_num++;

	ptr->prev=NULL;
	ptr->next=NULL;

	if(ptr->lock->tail) {
		ptr->prev = ptr->lock->tail;
		ptr->lock->tail->next=ptr;
		ptr->lock->tail=ptr;
	} else {
		ptr->lock->head=ptr->lock->tail=ptr;
	}
}

void remove_lock_node(lock_node_t *ptr) {
	ptr->lock->node_num--;

	if(ptr->lock->head == ptr->lock->tail) {
		ptr->lock->head = ptr->lock->tail = NULL;
	} else if(ptr->next == NULL) {
		ptr->prev->next = NULL;
		ptr->lock->tail = ptr->prev;
	} else if(ptr->prev == NULL) {
		ptr->next->prev = NULL;
		ptr->lock->head = ptr->next;
	} else {
		ptr->next->prev = ptr->prev;
		ptr->prev->next = ptr->next;
	}

	free(ptr);
}

void insert_conn_node(conn_node_t *ptr) {
	ptr->conn->node_num++;

	ptr->prev=NULL;
	ptr->next=NULL;

	if(ptr->conn->tail) {
		ptr->prev = ptr->conn->tail;
		ptr->conn->tail->next=ptr;
		ptr->conn->tail=ptr;
	} else {
		ptr->conn->head=ptr->conn->tail=ptr;
	}
}

void remove_conn_node(conn_node_t *ptr) {
	ptr->conn->node_num--;

	if(ptr->conn->head == ptr->conn->tail) {
		ptr->conn->head = ptr->conn->tail = NULL;
	} else if(ptr->next == NULL) {
		ptr->prev->next = NULL;
		ptr->conn->tail = ptr->prev;
	} else if(ptr->prev == NULL) {
		ptr->next->prev = NULL;
		ptr->conn->head = ptr->next;
	} else {
		ptr->next->prev = ptr->prev;
		ptr->prev->next = ptr->next;
	}

	free(ptr);
}
