#include "server.h"

//日志控制开关
int m_close_log=0;

WebServer::WebServer()
{
	base=event_init();

//Log Paremeters Set
	m_close_log=0;
	m_log_write=1;

}

WebServer::~WebServer()
{
	if(listen_ev!=NULL)
	{
		event_free(listen_ev);
	}
	if(base!=NULL)
	{
		event_base_free(base);
	}
}

void WebServer::init(const char* ip,short port)
{
//设置IP和Port
    if(ip==NULL)
	{
		printf("IP error and exit\n");
		exit(-1);
	}

    m_ip=ip;
	m_port=port;

//创建一个socket
    m_fd=socket( AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0 );
    if(-1 ==m_fd)
	{
		printf("create a socket failed and exit\n");
		exit(-1);
	}

//服务器绑定ip和port
    struct sockaddr_in local_addr; 
    memset(&local_addr,0,sizeof(local_addr)); 
    local_addr.sin_family=AF_INET; 
    local_addr.sin_addr.s_addr=inet_addr(m_ip);
    local_addr.sin_port=htons(m_port); 
    int bind_result = bind(m_fd, (struct sockaddr*) &local_addr, sizeof(struct sockaddr));
    if(bind_result < 0)
	{
        printf("bind system failed and exit\n");
		exit(-1);
    }

//监听port
    int ret=listen(m_fd,10);
	assert(ret!=-1);

}

void WebServer::handle_evtimer(int timer)
{
//暂时不设置
//	evhttp_set_timeout(http_server,timer);
}

void WebServer::handle_evio()
{

//主线程上的事件为“只读”
	listen_ev=event_new(base,m_fd,EV_READ|EV_PERSIST,Call_Back,this);
    event_add(listen_ev, NULL);
}

void WebServer::handle_evsignal()
{

}

void WebServer::event_loop()
{
	event_base_dispatch(base);
}

void WebServer::log_write()
{
	if(0==m_close_log)
	{
		if(1==m_close_log)
		Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 800);
		else
		Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 0);
	}
}

void WebServer::thread_pool(int thread_num)
{
	if(thread_num<=0)
	{
		printf("pthread number too small\n");
		thread_num=4;
	}
	m_thread_num=thread_num;

//线程池
   m_pool=new threadpool<NEW_FD>(m_thread_num);
}

void Call_Back(int sock, short event, void* arg)
{

//强制转换
    WebServer* server=(WebServer*)arg;

//加入工作队列
    server->m_pool->append_p(&(server->m_fd));
}

