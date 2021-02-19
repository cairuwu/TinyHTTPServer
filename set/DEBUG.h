#ifndef _CALL_BACK_
#define _CALL_BACK_

#include <event2/http.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http_struct.h>
#include <event2/http_compat.h>
#include <event2/util.h>

#include <sys/queue.h>
#include <event.h>
#include <string.h>  
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>

#include <string>

void HTTP_Call_Back(struct evhttp_request* request,void* arg);

void* read_process(struct evhttp_request* request);

void* write_process(struct evhttp_request* request);

std::string POST_parse(struct evhttp_request* request);

std::string GET_parse(struct evhttp_request* request);

#endif