#include "Wbase.c"
#include "Wsocket.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define INIT_DAEMON \
{ \
	if(fork() >0) exit(0); \
	setsid(); \
	if(fork() >0) exit(0); \
}

int main(void)
{
	int fd = open("/dev/zero", O_RDWR);
	dup2(fd,STDOUT_FILENO);
	signal(SIGPIPE, SIG_IGN);
	INIT_DAEMON
	Socket_destory(Server_start());
	return 0;
}