#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <signal.h>
#include "chttp.h"
#include "cutils.h"

/*server port*/
#define SERVER_PORT 8888

/*max file*/
#define FDS_MAXSIZE 1024

/*max events*/
#define MAXEVENTS 1024


/*客户端请求buf*/
#define BUFF_LEN 1024

typedef struct socket_info
{
	int cfd;
	int epfd;
} socket_info;

/*socket init*/
int Socket_init();

/*处理客户端任务*/
void *do_something(void* cfd);

/*服务器启动*/
int Server_start();

/*socket destory*/
int Socket_destory(int lsd);