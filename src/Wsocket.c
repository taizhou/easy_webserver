#include "Wbase.c"
#include "Wsocket.h"


int Socket_init()
{
	int lsd = 0;
	int ret = 0;
	struct sockaddr_in server_addr;
	int opt = 1;
	//创建套接字
	lsd = socket(AF_INET,SOCK_STREAM,0);
	if(lsd == -1)
		ERR_EXIT("socket");
	//端口复用
	setsockopt(lsd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//绑定端口
	ret = bind(lsd,(struct sockaddr*)&server_addr,sizeof(server_addr));
	if(ret == -1)
		ERR_EXIT("bind");
	//监听,建立tcp/ip协议栈
	ret = listen(lsd,SOMAXCONN);
	if(ret == -1)
		ERR_EXIT("bind");

	return lsd;
}

int Server_start()
{
	int lsd = 0 ,cfd = 0;//监听套接字
	int tmpfd = 0;//临时的描述符
	int epfd = 0;//epoll树根
	int ret = 0 ,i = 0;
	struct epoll_event lsdevent,tmpevent;
	struct epoll_event event[MAXEVENTS];
	struct sockaddr_in client_addr;
	socklen_t c_len = sizeof(client_addr);
	bzero(&client_addr,sizeof(client_addr));
	int nready = 0;
	lsd = Socket_init();


	epfd = epoll_create(FDS_MAXSIZE);
	if(epfd < 0)
		ERR_EXIT("epoll_create");

	//将监听套接字放入树中
	memset(&lsdevent,0,sizeof(lsdevent));
	memset(&tmpevent,0,sizeof(tmpevent));
	lsdevent.events = EPOLLIN;
	lsdevent.data.fd = lsd;
	ret = epoll_ctl(epfd,EPOLL_CTL_ADD,lsd,&lsdevent);
	if(ret < 0)
		ERR_EXIT("epoll_ctl");

	//初始化
	memset(event,0,sizeof(struct epoll_event)*MAXEVENTS);


	while(1)
	{
		nready = epoll_wait(epfd,event,MAXEVENTS,-1);
		if(nready <= 0)
			continue;

		for(i = 0; i < nready; i++)
		{
			tmpfd = event[i].data.fd;
			//有客户端访问
			if(tmpfd == lsd)
			{
				cfd = accept(lsd,(struct sockaddr*)&client_addr,&c_len);
				if(cfd < 0)
				{
					// if(errno == EAGAIN || errno == EINTR)
					// continue;
					continue;
				}

				//加入树中
				tmpevent.events = EPOLLIN;
				tmpevent.data.fd = cfd;
				ret = epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&tmpevent);
				if(ret < 0)
					continue;

				//printf("ip=%s,port=%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

			}
			else  //处理客户端发送的信息
			{
				//创建线程,传参
				socket_info *sinfo = (socket_info *)malloc(sizeof(socket_info));
				sinfo->cfd = tmpfd;
				sinfo->epfd = epfd;
				pthread_t pid;
				pthread_create(&pid,NULL,do_something,(void*)sinfo);
				pthread_detach(pid);
			}


		}
	}

	return lsd;
}


int Socket_destory(int lsd)
{
	close(lsd);
	return 0;
}


//读取客户端请求
int RecvRequest_Server(int sock,char* buf,size_t buflen)
{

	char* pTemp = buf;
	int ret = recv(sock,pTemp,buflen,0);
	if(ret == -1)
	{
		return -1;
	}
	return 0;
}


/*处理客户端任务*/
void *do_something(void* arg)
{
	socket_info *sinfo = (socket_info *)arg;

	char buf[BUFF_LEN] = {0};
	HTTP_Info* http = NULL;


	RecvRequest_Server(sinfo->cfd,buf,BUFF_LEN);

	//printf("客户端请求:\n");
	//printf("%s",buf);

	//如果发送的是空数据
	if(strcmp(buf,"") == 0)
	{
		//printf("请求为空!\n");
		//关闭连接
		shutdown(sinfo->cfd, SHUT_RDWR);
		goto End;
	}

	//解析HTTP报文
	http = ParseHttp_Server(buf,strlen(buf) + 1);

	char targetName[1024] = {0};
	IsExistRequestFile_Server(http->requesturl,targetName);
	//printf("1请求文件路径:%s\n",targetName);

	if(IsExcuteFile(targetName))
	{
		//printf("2请求文件路径:%s\n",targetName);
		//开子进程执行其他应用程序
		pid_t pid = vfork();
		if(pid < 0)
		{
			perror("fork error:");
		}
		if(pid == 0)
		{
			char sockstr[12] = {0};
			sprintf(sockstr,"%d",sinfo->cfd);
			if (execl(targetName,sockstr,http->requestdata,NULL) < 0 )
			{
				//perror("execl error:");
				//printf("执行程序失败!\n");
			}
		}
	}
	else
	{
		//printf("3请求文件路径:%s\n",targetName);
		//读取资源文件 返回
		Cstring* data = ReadFileContent_Utils(targetName,http);
		send(sinfo->cfd,data->str,data->length,0);
		Destroy_Cstring(data);
	}
End:
	close(sinfo->cfd);

	free(sinfo);
	sinfo = NULL;

	if(http)
		DestroyHTTP_Server(http);
	
	//printf("客户端关闭\n");
	return NULL;
}