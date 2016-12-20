#ifndef HAVE_LES_H
#define HAVE_LES_H

#include <event.h>
#include <pthread.h>

#include "conn.h"

#if 1
	#define dprintf(...) printf(__VA_ARGS__)
#else
	#define dprintf(...)
#endif

#define LES_HOST "0.0.0.0"
#define LES_PORT 1111
#define LES_BACKLOG 10
#define LES_MAX_CONN 100
#define LES_BUF_SIZE 1024

typedef struct
{
	struct event_base *base;
	struct event event;
	struct event signal_int;
	struct event timeout;

	int listen_fd;

	short  ev_flags;
} listen_thread_t;

extern listen_thread_t listen_thread;

#endif