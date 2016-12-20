#include <stdlib.h>
#include "conn.h"

conn_t *head_conn = NULL;
conn_t *tail_conn = NULL;
unsigned int conn_num = 0;

void insert_conn(conn_t *ptr) {
	conn_num++;

	ptr->next=NULL;

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

	shutdown(ptr->sockfd,2);
	close(ptr->sockfd);

	event_del(&ptr->event);

	free(ptr);
}

void clean_conn() {
	conn_t *ptr=head_conn,*tmp;
	while(ptr) {
		shutdown(ptr->sockfd,2);
		close(ptr->sockfd);

		event_del(&ptr->event);

		tmp=ptr;
		ptr=ptr->next;

		free(tmp);
	}

	conn_num = 0;
}
