#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <signal.h>
#include <sys/errno.h>
#include <curses.h>
#include <sys/socket.h>
#include <error.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <glib.h>

#include "conn.h"
#include "dls.h"

listen_thread_t listen_thread;

GHashTable *ht_lock = NULL;

static void listen_handler(const int fd, const short which, void *arg);
inline bool update_accept_event(const int new_flags) {
	if(listen_thread.ev_flags==new_flags) {
		return false;
	}

    if (event_del(&listen_thread.event) == -1) {
		return false;
	}

	listen_thread.ev_flags=new_flags;

	event_set(&listen_thread.event, listen_thread.listen_fd, new_flags, listen_handler, NULL);
	event_base_set(listen_thread.base, &listen_thread.event);
    event_add(&listen_thread.event, NULL);

	return true;
}

inline void is_accept_conn(bool do_accept) {
	if (do_accept) {
		if (update_accept_event(EV_READ | EV_PERSIST) && listen(listen_thread.listen_fd, LES_BACKLOG) != 0) {
			perror("listen");
		}
	}
	else {
		if (update_accept_event(0) && listen(listen_thread.listen_fd, 0) != 0) {
			perror("listen");
		}
	}
}

inline void send_status(int sockfd, bool status) {
	char b = '0';
	
	if(status) {
		b = '1';
	}

	send(sockfd, &b, 1, MSG_WAITALL);
}

static void read_handler(int sock, short event,	void* arg)
{
	conn_t *ptr = (conn_t *) arg;

	dprintf("%s: sockfd(%d), host(%s), port(%d)!\n", __func__, ptr->sockfd, ptr->host, ptr->port);

	register int ret;
	char data[LES_BUF_SIZE+2];

	ret=recv(ptr->sockfd, data, LES_BUF_SIZE, 0);
	if(ret<=0){//关闭连接
		dprintf("%s(socket_close): sockfd(%d), host(%s), port(%d)!\n", __func__, ptr->sockfd, ptr->host, ptr->port);

		if(head_conn == ptr && ptr->next) {
			send_status(ptr->next->sockfd, true);
		}

		remove_conn(ptr);

		is_accept_conn(true);
	}else{//接收数据成功
		data[ret] = '\0';
		data[ret+1] = '\0';
		register char *s=data;
		register size_t l;
		register conn_node_t *p;
		register lock_t *lock;
		
		GString str={NULL,0,0};

		while(*s) {
			l=strlen(s);
			dprintf("key: %s\n", s);
			switch(*s) {
				case '*': {
					p=ptr->head;
					str.len=0;
					while(p) {
						lock=p->node->lock;
						if(str.len>0) {
							g_string_append_c(&str,',');
						}
						g_string_append_printf(&str, "%s(%d)", lock->key, lock->head==p->node?1:0);
						p=p->next;
					}
					g_string_append_c(&str,'\n');
					dprintf("%s: %s\n", __func__, str.str);
					if(str.len>0) {
						send(ptr->sockfd, str.str, str.len, MSG_WAITALL);
					}
					break;
				}
				case '+': {
					p=g_hash_table_lookup(ptr->ht_lock_node,s+1);
					if(p) {
						lock=p->node->lock;
					} else {
						lock=g_hash_table_lookup(ht_lock,s+1);
						if(!lock) {
							lock=(lock_t*)malloc(sizeof(lock_t));
							bzero(lock,sizeof(lock_t));
							lock->key = strdup(s+1);
							g_hash_table_insert(ht_lock,lock->key,lock);
						}

						p=(conn_node_t*)malloc(sizeof(conn_node_t));
						p->conn=ptr;
						p->node=(lock_node_t*)malloc(sizeof(lock_node_t));
						p->node->lock=lock;
						p->node->conn=ptr;

						insert_lock_node(p->node);
						insert_conn_node(p);

						g_hash_table_insert(ptr->ht_lock_node, lock->key, p);

					}

					send_status(ptr->sockfd, lock->head==p->node);
					break;
				}
				case '-': {
					p=g_hash_table_lookup(ptr->ht_lock_node,s+1);
					if(p) {
						lock=p->node->lock;

						if(p->node==lock->head && lock->head->next) {
							send_status(lock->head->next->conn->sockfd, true);
						}

						g_hash_table_remove(ptr->ht_lock_node, lock->key);

						remove_lock_node(p->node);
						remove_conn_node(p);
					}
					send_status(ptr->sockfd, p != NULL);
					break;
				}
			}
			s+=l+1;
		}
		if(str.str) {
			free(str.str);
		}
	}
}

static void listen_handler(const int fd, const short which, void *arg)
{
	char buf[1];
	int conn_fd;
	struct sockaddr_in pin;
	socklen_t len=sizeof(pin);

	conn_fd=accept(listen_thread.listen_fd,(struct sockaddr *)&pin,&len);
	
	if(conn_fd<=0){
		return;
	}

	conn_t *ptr=(conn_t *)malloc(sizeof(conn_t));

	bzero(ptr,sizeof(conn_t));

	ptr->sockfd=conn_fd;
	inet_ntop(AF_INET,&pin.sin_addr,ptr->host,sizeof(ptr->host));
	ptr->port=ntohs(pin.sin_port);

	if(conn_num >= LES_MAX_CONN) {
		is_accept_conn(false);

		shutdown(ptr->sockfd,2);
		close(ptr->sockfd);

		dprintf("%s(denied): sockfd(%d), host(%s), port(%d)!\n", __func__, ptr->sockfd, ptr->host, ptr->port);
	} else {
		event_set(&ptr->event, ptr->sockfd, EV_READ|EV_PERSIST, read_handler, ptr);
		event_base_set(listen_thread.base, &ptr->event);
		event_add(&ptr->event, NULL);
		
		if(head_conn == NULL) {
			send_status(ptr->sockfd, true);
		}

		insert_conn(ptr);

		dprintf("%s(accepted): sockfd(%d), host(%s), port(%d)!\n", __func__, ptr->sockfd, ptr->host, ptr->port);
	}
}

static void signal_handler(const int fd, short event, void *arg)
{
	struct event *signal = arg;

	dprintf("\n\n%s: got signal %d\n", __func__, EVENT_SIGNAL(signal));

	event_del(signal);

	is_accept_conn(false);

	dprintf("%s: exit main thread\n", __func__);
	event_base_loopbreak(listen_thread.base);
}

static void timeout_handler(const int fd, short event, void *arg)
{
	conn_t *ptr = head_conn;

	while(ptr) {
		send_status(ptr->sockfd, ptr->tail && ptr->tail->node == ptr->tail->node->lock->head);
		ptr = ptr->next;
	}
}

static void free_lock(lock_t *ptr) {
	dprintf("%s(%s: %d).\n", __func__, ptr->key, ptr->node_num);
	free(ptr);
}

int main(int argc, char *argv[]) {
	struct sockaddr_in sin;
	int listen_fd;
	int ret;

	listen_fd=socket(AF_INET,SOCK_STREAM,0);
	if(listen_fd<0){
		dprintf("Not on the host %s bind port %d\n",LES_HOST,LES_PORT);
		return 0;
	}
	int opt=1;
	setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int));

	int send_timeout=5000,recv_timeout=5000;
	setsockopt(listen_fd,SOL_SOCKET,SO_SNDTIMEO,&send_timeout,sizeof(int));//发送超时
	setsockopt(listen_fd,SOL_SOCKET,SO_RCVTIMEO,&recv_timeout,sizeof(int));//接收超时

	typedef struct {
		u_short l_onoff;
		u_short l_linger;
	} linger;
	linger m_sLinger;
	m_sLinger.l_onoff=1;//(在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
	// 如果m_sLinger.l_onoff=0;则功能和2.)作用相同;
	m_sLinger.l_linger=5;//(容许逗留的时间为5秒)
	setsockopt(listen_fd,SOL_SOCKET,SO_LINGER,(const char*)&m_sLinger,sizeof(linger));

	int send_buffer=0,recv_buffer=0;
	setsockopt(listen_fd,SOL_SOCKET,SO_SNDBUF,(char *)&send_buffer,sizeof(int));//发送缓冲区大小
	setsockopt(listen_fd,SOL_SOCKET,SO_RCVBUF,(char *)&recv_buffer,sizeof(int));//接收缓冲区大小

	bzero(&sin,sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr=inet_addr(LES_HOST);
	sin.sin_port=htons(LES_PORT);

	ret=bind(listen_fd,(struct sockaddr *)&sin,sizeof(sin));
	if(ret<0){
		dprintf("Not on the host %s bind port %d\n",LES_HOST,LES_PORT);
		return 0;
	}

	ret=listen(listen_fd, LES_BACKLOG);
	if(ret<0){
		system("echo -e \"\\E[31m\".[Failed]");
		system("tput sgr0");
		return 0;
	}

	// init main thread
	listen_thread.listen_fd = listen_fd;
	listen_thread.base = event_init();
	if (listen_thread.base == NULL)
	{
		perror("event_init( base )");
		exit(1);
	}

	// listen event
	event_assign(&listen_thread.event, listen_thread.base, listen_fd, EV_READ|EV_PERSIST, listen_handler, NULL);
	event_add(&listen_thread.event, NULL);

	// int signal event
	event_assign(&listen_thread.signal_int, listen_thread.base, SIGINT, EV_SIGNAL|EV_PERSIST, signal_handler, &listen_thread.signal_int);
	event_add(&listen_thread.signal_int, NULL);

#ifdef DLS_TIMEOUT
	struct timeval tv;
	bzero(&tv,sizeof(tv));
	tv.tv_sec = 5;

	event_set(&listen_thread.timeout, -1, EV_TIMEOUT|EV_PERSIST, timeout_handler, NULL);
	event_base_set(listen_thread.base, &listen_thread.timeout);
	event_add(&listen_thread.timeout, &tv);
#endif

	dprintf("started.\n");
	ht_lock = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify)free_lock);

	event_base_loop(listen_thread.base, 0);

	clean_conn();

	g_hash_table_destroy(ht_lock);
	
#ifdef DLS_TIMEOUT
	event_del(&listen_thread.timeout);
#endif
	event_del(&listen_thread.signal_int);
	event_del(&listen_thread.event);
	event_base_free(listen_thread.base);

	dprintf("exited.\n");

	return 0;
}
