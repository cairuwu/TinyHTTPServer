#include "callback.h"

//数据库的相关设置
std::string m_user="root";
std::string m_passWord="root";
std::string m_databaseName="yourdb";
int m_sql_num=8;

connection_pool *m_connPool=NULL;

//map来存储用户和密码
locker m_lock;
std::map<std::string, std::string> users;

//文件所放的位置
const char* doc_root="/home/cairuwu/HTTP_Server/root";

void Add_Content_Type(struct evhttp_request* request,std::string s)
{
    evhttp_add_header(request->output_headers,"Content-Type","text/html; charset=UTF-8");
}

void Add_Connection(struct evhttp_request* request,std::string s)
{
    evhttp_add_header(request->output_headers,"Connection",s.c_str());
}

//从数据库中取出一条连接
void Initmysql(connection_pool* connpool)
{

 //先从连接池中取一个连接
    MYSQL *mysql = NULL;
    connectionRAII mysqlcon(&mysql, m_connPool);

    //在user表中检索username，passwd数据，浏览器端输入
    if (mysql_query(mysql, "SELECT username,passwd FROM user"))
    {
        printf("SELECT error:%s\n", mysql_error(mysql));
    }

    //从表中检索完整的结果集
    MYSQL_RES *result = mysql_store_result(mysql);

    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);

    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);

    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = mysql_fetch_row(result))
    {
        string temp1(row[0]);
        string temp2(row[1]);
        users[temp1] = temp2;
    }

}

//数据库的设置
void sql_pool()
{
//数据库初始化
    m_connPool=connection_pool::GetInstance();
    m_connPool->init("localhost", m_user, m_passWord, m_databaseName, 3306, m_sql_num);

//初始化读取数据库表
   Initmysql(m_connPool);
}

std::string POST_parse(struct evhttp_request* request)
{

   char temp[1024]="\0";
   
    int length= EVBUFFER_LENGTH(request->input_buffer);
    int ret=evbuffer_remove(request->input_buffer,temp,1024);
    //sprintf(test,"POST(origin): %s\n",post_data);
    printf("remove data: %d\n",ret);
    printf("%s",temp);


    
    printf("POST length: %d\n",length);

    if(length==0)
    return std::string();

    return std::string(temp);
}

//POST报文的验证
void Content_parse(string& URL,string& POST,string& file_name,int cgi)
{

//根据URL来判断是登录还是注册
    const char *p = strrchr(URL.c_str(), '/');

    if (strlen(URL.c_str()) == 1)
    {
        file_name="/judge.html";
        return;
    }

     if (cgi == 1 && (*(p + 1) == '2' || *(p + 1) == '3'))
    {
//提交用户名和登录密码
//先从连接池中取一个连接
    MYSQL *mysql = NULL;
    connectionRAII mysqlcon(&mysql, m_connPool);

    //在user表中检索username，passwd数据，浏览器端输入
    if (mysql_query(mysql, "SELECT username,passwd FROM user"))
    {
        printf("SELECT error:%s\n", mysql_error(mysql));
    }

    //从表中检索完整的结果集
    MYSQL_RES *result = mysql_store_result(mysql);

    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);

    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);

    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = mysql_fetch_row(result))
    {
        string temp1(row[0]);
        string temp2(row[1]);
        users[temp1] = temp2;
    }

    const char* m_string=POST.c_str();
    char name[100], password[100];

    int i;
    for (i = 5; m_string[i] != '&'; ++i)
        name[i - 5] = m_string[i];
      name[i - 5] = '\0';

    printf("user: %s\n",name);

    int j = 0;
    for (i = i + 10; m_string[i] != '\0'; ++i, ++j)
        password[j] = m_string[i];
    password[j] = '\0';
    printf("password: %s\n",password);
//注册
    if (*(p + 1) == '3')
    {
            //如果是注册，先检测数据库中是否有重名的
            //没有重名的，进行增加数据
            char *sql_insert = (char *)malloc(sizeof(char) * 200);
            strcpy(sql_insert, "INSERT INTO user(username, passwd) VALUES(");
            strcat(sql_insert, "'");
            strcat(sql_insert, name);
            strcat(sql_insert, "', '");
            strcat(sql_insert, password);
            strcat(sql_insert, "')");

            if (users.find(name) == users.end())
            {

            //从数据库中取出一条连接
            //并采用RTTI机制进行管理
            //    MYSQL* mysql=NULL;
            //    connectionRAII mysqlcon(&mysql, m_connPool);

                int res = mysql_query(mysql, sql_insert);
                m_lock.lock();
                users.insert(pair<string, string>(name, password));
                m_lock.unlock();

                if (!res)
                    file_name="/log.html";
                else
                    file_name="/registerError.html";
            }
            else
                file_name="/registerError.html";

            free(sql_insert);
            return;
    }
    else if (*(p + 1) == '2')
    {
            if (users.find(name) != users.end() && users[name] == password)
                file_name="/welcome.html";
            else
                file_name="/logError.html";

            return;
    }
    }

    if (*(p + 1) == '0')
    {
        file_name="/register.html";
    }
    else if (*(p + 1) == '1')
    {
        file_name="/log.html";
    }
    else if (*(p + 1) == '5')
    {
        file_name="/picture.html";
    }
    else if (*(p + 1) == '6')
    {
        file_name="/video.html";
    }
    else if (*(p + 1) == '7')
    {
        file_name="/fans.html";
    }

}


//实际上是解析URL
std::string URL_parse(struct evhttp_request* request)
{
    if(request->uri==NULL)
    {
        printf("URL is empty\n");
        return std::string();
    }
    return std::string(request->uri);
}

//添加对应的文件
void Add_File(struct evhttp_request* request,const char* file_name)
{
    if(!file_name)
    {
        printf("file pointer is null\n");
        return;
    }


    char m_real_file[2048]="\0";
    strcpy(m_real_file, doc_root);
    strcat(m_real_file, file_name);

//DEBUG
    printf("source:%s \n",m_real_file);

    struct stat m_file_stat;

    if (stat(m_real_file, &m_file_stat) < 0)
    {
        printf("no request source \n");
        return ;
    }

    if (!(m_file_stat.st_mode & S_IROTH))
    {
        printf("forbidden source\n");
        return ;
    }

    if (S_ISDIR(m_file_stat.st_mode))
    {
        printf("bad request\n");
        return ;
    }

//Memory mmap
    int fd = open(m_real_file, O_RDONLY);
    char* m_file_address = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

//libevent buffer
    struct evbuffer* evbuf = evbuffer_new();

//put file into buffer
    evbuffer_add(evbuf,m_file_address,m_file_stat.st_size);

//HTTP response 200 OK 
    evhttp_send_reply(request, HTTP_OK, "OK", evbuf);

//free buffer
    evbuffer_free(evbuf);

//Memory unmmap
    if(m_file_address)
    {
        munmap(m_file_address,m_file_stat.st_size);
        m_file_address=0;
    }
}

void HTTP_Call_Back(struct evhttp_request* req,void* arg)
{
    
//"读"处理
    std::string URL=URL_parse(req);
    std::string POST=POST_parse(req);

    std::cout<<"URL is"<<URL<<"\n";
    std::cout<<"URL length: "<<URL.size()<<"\n";

    //std::cout<<"POST data: "<<POST<<"\n";

    std::string file_name;

    int cgi=0;
//GET报文(实体为空)
    if(POST.empty())
    {
        printf("GET response\n");
        Content_parse(URL,POST,file_name,cgi);
        //GET_response(URL,file_name);
    }
    else
    {
//POST报文
        cgi=1;
        printf("POST reponse\n");
        printf("POST: %s\n",POST.c_str());

        Content_parse(URL,POST,file_name,cgi);
    }


//"写"处理
    write_process(req,URL,file_name);
}

void write_process(struct evhttp_request* req,std::string & URL,std::string& file_name)
{

/*BEBUG
    if(URL.empty())
    {
        printf("URL Is Empty\n");
        return;
    }
*/
    if(file_name.empty())
    {

//找不到文件时的HTTP响应
    struct evbuffer *buf;
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "URL Is Wrong! And Wrong URL Is: %s\n",URL.c_str());
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
    }

    std::string Type="text/html";
    std::string Connection("close");

    Add_Content_Type(req,Type);
    Add_File(req,file_name.c_str());
    
}
