#ifndef _CAIRUWU_SERVER_
#define _CAIRUWU_SERVER_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event.h>
#include <event2/http.h>
#include <event2/http_struct.h>

#include "./log/log.h"
#include "./threadpool/threadpool.h"

#define NEW_FD int
//回调函数
void Call_Back(int sock, short event, void* arg);


class WebServer
{
public:
	WebServer();
	~WebServer();
	void init(const char* ip,short port);

//events
	void handle_evtimer(int time);

//主线程的事件只设置为“读”事件，由回调函数调用子进程处理
	void handle_evio();

	void handle_evsignal();

//dispatch
	void event_loop();

//Log init
   void log_write();

//threadpool
   void thread_pool(int thread_num);


public:

//Libevent
   struct event_base* base;
	struct event* listen_ev;

//Log
//0:synchronous  and 1:asynchronous
    int m_close_log;
	 int m_log_write;

//thread_pool Set
   threadpool<NEW_FD> *m_pool;
   int m_thread_num;

//listen port and m_fd
   const char* m_ip;
   int m_fd;
   int m_port;
   int m_epollfd;

};

#endif
