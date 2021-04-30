#include "server.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <ctype.h>      //toupper的头文件
#include <arpa/inet.h>  // sockaddr_in
#include "ThreadPool.h"
#include "errno.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <strings.h>
#include "Log.h"

#include <sys/epoll.h>
#include <sys/resource.h>
#include <fcntl.h>

#define MAX_FDS            10240 //10240
#define MAX_EVENTS         MAX_FDS

void *test(void *Arg)
{
    if ( !Arg ) return NULL;
   
    uint8_t buf[1024] = {0};
  
    int client_fd = *((int *)Arg);

    while (1)
    {
      // LOG("%d, %lx", client_fd, pthread_self());
        int n = read(client_fd, buf, sizeof(buf));
        if ( n <= 0 )
        {
            LOG(" %lx exit ",  pthread_self());
            close(client_fd);
            return NULL;
        }
        LOG("%s", buf);
		write(client_fd, buf, n);
    }

    close(client_fd);

    return NULL;
}

ThreadPool_t *Pool = NULL;
int fd = -1;
void Stop(int signo) 
{
    ThreadPoolDestroy(Pool);
    Pool = NULL;
    close(fd);
    exit(0);
}

int main (int argc, char ** argv)
{
    if ( argc < 2 )
    {
        ERROR("Please Input port");
        return 0;
    }

    /*修改文件描述符限制*/
	struct rlimit FdLimit = {0};

	FdLimit.rlim_cur = MAX_EVENTS;
	FdLimit.rlim_max = MAX_EVENTS;

	setrlimit(RLIMIT_NOFILE, &FdLimit);
	getrlimit(RLIMIT_NOFILE, &FdLimit);

    LOG("Cur : %d \n Max : %d", (int)FdLimit.rlim_cur, (int)FdLimit.rlim_max);

    signal(SIGINT, Stop); 
	if ( !(Pool = PoolInit(1 ,40, 100)) ) 
    {
		ERROR("threadpool_create false\n");
		return -1;
	}

    /*IPV4 , TCP, defult protocol*/
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( fd < 0 )
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in serv_addr, client_addr;
    socklen_t socklen;

    bzero(&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[1]));  /* 指定端口号 */
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* 自动选择可用IP */

    int on = 1;
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ); //快速重用

	if ( bind(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr) ) < 0) {
		perror("bind");
        close(fd);
        ThreadPoolDestroy(Pool);
        return -1;
	}

	if ( listen(fd, 10) < 0 ) {
		perror("listen");
        ThreadPoolDestroy(Pool);
        close(fd);
        return -1;
	}

    char buf[100] ={0};
    socklen = sizeof(client_addr);

    for(;;)
    {

       	int clientfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
		if (-1 == clientfd) {
			if (EINTR == errno)
				continue;
			perror("accept error.");
			return -1;
		}

        inet_ntop(AF_INET, &client_addr, buf, INET_ADDRSTRLEN);
	    LOG("client IP is: %s, client port is: %d, socket_fd addr = %p\n", buf, ntohs(client_addr.sin_port), &clientfd);

		 int rc = AddTask(Pool, test, (void *)&clientfd);
		 if (rc < 0) {
            ThreadPoolDestroy(Pool);
            close(fd);
			ERROR("threadpool_create false");
			return -1;
		}
    }

	ThreadPoolDestroy(Pool);
    close(fd);

    return 0;
}