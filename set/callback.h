#ifndef _CALL_BACK_
#define _CALL_BACK_

//libevent
#include <event2/http.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http_struct.h>
#include <event2/http_compat.h>
#include <event2/util.h>
//output
#include <sys/queue.h>
#include <event.h>
#include <string.h>  
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
//system control
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>

//mysql
#include <mysql/mysql.h>

#include <map>
#include <string>
#include <iostream>

#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"

#define MYHTTPD_SIGNATURE   "myhttpd v 0.0.1"

void HTTP_Call_Back(struct evhttp_request* request,void* arg);

std::string read_process(struct evhttp_request* request);

void write_process(struct evhttp_request* req,std::string & URL,std::string& file_name);
 

std::string POST_parse(struct evhttp_request* request);

std::string URL_parse(struct evhttp_request* request);

void Add_Content_Type(struct evhttp_request* request,std::string s);

void Add_Connection(struct evhttp_request* request,std::string s);

void Add_File(struct evhttp_request* request,const char* file_name);

void Content_parse(const string& URL,string& POST,string& file_name,int cgi);

void Initmysql(connection_pool* connpool);

void sql_pool();

#endif