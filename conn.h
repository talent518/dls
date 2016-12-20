#ifndef _CONN_H
#define _CONN_H

#include <stdbool.h>
#include <event.h>

typedef struct _conn_t{
	int sockfd;
	char host[15];
	int port;

	struct event event;

	struct _conn_t *prev;
	struct _conn_t *next;
} conn_t;

extern conn_t *head_conn;
extern conn_t *tail_conn;
extern unsigned int conn_num;

void insert_conn(conn_t *ptr);
void remove_conn(conn_t *ptr);
void clean_conn();

#endif