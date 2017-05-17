#ifndef HAVE_DLS_H
#define HAVE_DLS_H

#include <event.h>
#include <pthread.h>

#include "config.h"

#include "conn.h"

#ifdef DLS_DEBUG
	#define dprintf(...) printf(__VA_ARGS__)
#else
	#define dprintf(...)
#endif

#define LES_HOST "0.0.0.0"
#define LES_PORT 1112
#define LES_BACKLOG 10
#define LES_MAX_CONN 100
#define LES_BUF_SIZE 1024

typedef struct
{
	struct event_base *base;
	struct event event;
	struct event signal_int;
	struct event timeout;
	struct event timeout_clean;

	int listen_fd;

	short  ev_flags;
} listen_thread_t;

extern listen_thread_t listen_thread;

extern GHashTable *ht_lock;

#endif
