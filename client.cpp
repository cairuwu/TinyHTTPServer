#define  _GNU_SOURCE 1//为了支持POLLRDHUP事件  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <assert.h>  
#include <stdio.h>  
#include <unistd.h>  
#include <string.h>  
#include <stdlib.h>  
#include <poll.h>  
#include <fcntl.h>  
#include <iostream>  
#include <ctime>
#include <sys/epoll.h>
#define BUFFER_SIZE 2000 
#define MAX_EVENTS 1024 
using namespace std;  

int setNonBlocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addfd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP;// | EPOLLERR | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);//第三个参数提供需要注册的fd,第四个参数&event提供所注册的事件
    setNonBlocking(fd);
}

int main(int argc,char* argv[])
{  
    if(argc<=2)
    {  
        cout<<"argc <= 2"<<endl;  
        return 1;  
    }

    const char* ip = argv[1];//服务端地址  
    int port = atoi(argv[2]);  

    struct sockaddr_in address;  
    bzero(&address,sizeof(address)); 

    address.sin_family = AF_INET;  
    inet_pton(AF_INET,ip,&address.sin_addr);  
    address.sin_port = htons(port);  

    int sockfd = socket(PF_INET,SOCK_STREAM,0);  
    assert(sockfd >= 0); 

    if(connect(sockfd,(struct sockaddr*)&address,sizeof(address))<0)
    {  
        cout<<"connect error"<<endl;  
        close(sockfd);  
        return 1;  
    } 
    socklen_t server_len = sizeof(address);
    cout << "connect to server " << argv[1] << " success" << endl;

    epoll_event client[MAX_EVENTS];
    int epollfd = epoll_create(5);

    addfd(epollfd, sockfd);//注册sockfd和标准输入STD_IN
    addfd(epollfd, STDIN_FILENO);

    time_t rawtime;
    struct tm * timeinfo;

    char read_buf[BUFFER_SIZE];  
    int pipefd[2];  
    int ret = pipe(pipefd);//创建一个管道，splice函数的参数必须有个是管道描述符(实现零拷贝)  
    assert(ret != -1);  

    while(1)
    {  
        ret = epoll_wait(epollfd, client, MAX_EVENTS, -1);//无限期等待注册事件发生  100是最大监听事件数
        if(ret < 0)
        {  
            cout<<"epoll error"<<endl;  
            close(sockfd); 
            return 0;  
        }  
        for(int i = 0; i < ret; ++i)
        {
            if(client[i].events & EPOLLRDHUP)
            {//若是socket描述符挂起事件则代表服务器关闭了连接  
                cout<<"server close the connection"<<endl;  
                close(sockfd);
                return 0;  
            }  
            else if(client[i].events & EPOLLIN)
            {
                if(client[i].data.fd == STDIN_FILENO)//STDIN_FILENO)
                {//标准输入端可写事件发生(该用户有数据输入并需要发送给服务端)  
                    ret=splice(STDIN_FILENO, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE|SPLICE_F_MOVE);//将标准输入的数据零拷贝到管道的写端  
                    ret=splice(pipefd[0], NULL, sockfd, NULL, 32768, SPLICE_F_MORE|SPLICE_F_MOVE);//将管道的读端数据零拷贝到socket描述符  

                    time ( &rawtime );
                    timeinfo = localtime ( &rawtime );
                    cout<<" Message send time is " << asctime (timeinfo);
                }
                else
                {//sokect描述符可读事件 不管是用户输入还是服务器输入，均为EPOLLIN事件 
                    memset(read_buf,'\0',BUFFER_SIZE);  
                    recv(client[i].data.fd, read_buf, BUFFER_SIZE-1, 0);//接收服务端发送来的数据(服务端的数据是其它用户发送给它的数据)  

                    // time_t rawtime;
                    // struct tm * timeinfo;
                   time ( &rawtime );
                   timeinfo = localtime ( &rawtime );
                   cout<<"Message receive time is " << asctime (timeinfo);
                    // printf ( "当前系统时间: %s", asctime (timeinfo) ); 
                    cout<< read_buf;
                }  
            }
        }  
    }  
    return 0;  
}  
