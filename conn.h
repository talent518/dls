#ifndef HAVE_CONN_H
#define HAVE_CONN_H

#include <stdbool.h>
#include <event.h>
#include <glib.h>

typedef struct _conn_t{
	int sockfd;
	char host[16];
	int port;

	struct event event;

	struct _conn_t *prev;
	struct _conn_t *next;

	GHashTable *ht_lock_node;

	unsigned int node_num;

	struct _conn_node_t *head;
	struct _conn_node_t *tail;
} conn_t;

typedef struct _conn_node_t{
	struct _conn_t *conn;
	struct _lock_node_t *node;

	struct _conn_node_t *prev;
	struct _conn_node_t *next;
} conn_node_t;

typedef struct _lock_t{
	unsigned int node_num;

	char *key;

	struct _lock_node_t *head;
	struct _lock_node_t *tail;
} lock_t;

typedef struct _lock_node_t{
	struct _lock_t *lock;
	struct _conn_t *conn;

	struct _lock_node_t *prev;
	struct _lock_node_t *next;
} lock_node_t;

extern conn_t *head_conn;
extern conn_t *tail_conn;
extern unsigned int conn_num;

void insert_conn(conn_t *ptr);
void remove_conn(conn_t *ptr);
void clean_conn();

void insert_lock_node(lock_node_t *ptr);
void remove_lock_node(lock_node_t *ptr);
void insert_conn_node(conn_node_t *ptr);
void remove_conn_node(conn_node_t *ptr);

#endif
