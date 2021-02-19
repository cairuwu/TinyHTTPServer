#include "server.h"

const char* ip="192.168.137.150";
short port=5555;
int slot=5;

int main()
{
	WebServer server;

//mysql数据库的初始化
    sql_pool();
	
	server.log_write();

	server.init(ip,port);

//设置主线程的“读”事件
    server.handle_evio();

//设置线程池
    server.thread_pool(1);

//Loop
	server.event_loop();

	return 0;
}
