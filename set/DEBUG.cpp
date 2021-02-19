#include "callback.h"
//文件所放的位置
const char* doc_root="/home/cairuwu/HTTP_Server/root";

#define MYHTTPD_SIGNATURE   "myhttpd v 0.0.1"
void HTTP_Call_Back(struct evhttp_request* req,void* arg)
{
    
//"读"处理
    read_process(req);

//"写"处理
    write_process(req);
}

void* write_process(struct evhttp_request* req)
{
    
    //获取POST方法的数据
    char *post_data = (char *) EVBUFFER_DATA(req->input_buffer);
    printf("POST: %s\n",post_data);

    //HTTP header
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/html; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "keep-alive");

    if(!req)
    {
        printf("http request is NULL \n");
        return NULL;
    }

    char m_real_file[2048]="\0";
    strcpy(m_real_file, doc_root);
    strcat(m_real_file, "/register.html");

    printf("source:%s \n",m_real_file);

    struct stat m_file_stat;

    if (stat(m_real_file, &m_file_stat) < 0)
    {
        printf("no request source \n");
        return NULL;
    }

    if (!(m_file_stat.st_mode & S_IROTH))
    {
        printf("forbidden source\n");
        return NULL;
    }

    if (S_ISDIR(m_file_stat.st_mode))
    {
        printf("bad request\n");
        return NULL;
    }

    int fd = open(m_real_file, O_RDONLY);
    char* m_file_address = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    //struct evkeyvalq params;
    struct evbuffer* evbuf = evbuffer_new();
   // evbuffer_add_printf(evbuf, "Thanks for the request!");
    evbuffer_add(evbuf,m_file_address,m_file_stat.st_size);
    //evhttp_add_header(request->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_send_reply(req, HTTP_OK, "OK", evbuf);
    evbuffer_free(evbuf);

}

void* read_process(struct evhttp_request* req)
{
    char output[2048] = "\0";
    char tmp[1024];

    //获取客户端请求的URI(使用evhttp_request_uri或直接req->uri)
    const char *uri;
    uri = evhttp_request_uri(req);
    sprintf(tmp, "uri=%s\n", uri);
    strcat(output, tmp);

    sprintf(tmp, "uri=%s\n", req->uri);
    strcat(output, tmp);
    //decoded uri
    char *decoded_uri;
    decoded_uri = evhttp_decode_uri(uri);
    sprintf(tmp, "decoded_uri=%s\n", decoded_uri);
    strcat(output, tmp);

    //解析URI的参数(即GET方法的参数)
    struct evkeyvalq params;
    //将URL数据封装成key-value格式,q=value1, s=value2
    evhttp_parse_query(decoded_uri, &params);
    //得到q所对应的value
    sprintf(tmp, "q=%s\n", evhttp_find_header(&params, "q"));
    strcat(output, tmp);
    //得到s所对应的value
    sprintf(tmp, "s=%s\n", evhttp_find_header(&params, "s"));
    strcat(output, tmp);

    free(decoded_uri);

}