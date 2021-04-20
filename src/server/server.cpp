#include "server.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <ctype.h>  //toupper的头文件
#include <arpa/inet.h>  // sockaddr_in
extern "C" 
{
    #include "ThreadPool.h"
}
#include "errno.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <strings.h>
#include "Log.h"


void *test(void *Arg)
{
    if ( !Arg ) return NULL;
   
    uint8_t buf[1024] = {0};
  
    int client_fd = *((int *)Arg);

    while (1)
    {
        LOG("%d, %lx", client_fd, pthread_self());
        int n = read(client_fd, buf, sizeof(buf));
        if ( n <= 0 )
        {
            LOG(" %lx exit ",  pthread_self());
            close(client_fd);
            return NULL;
        }
		write(client_fd, buf, n);
    }

    close(client_fd);

    return NULL;
}

void *pool;
int fd = -1;
void Stop(int signo) 
{
	//threadpool_destroy(pool, FLAGS_WAIT_TASK_EXIT);
    LOG("ccc");
    close(fd);
    exit(0);
}

int main (int argc, char ** argv)
{
    if ( argc < 2 )
    {
        ERROR("Please Input xxx [port]");
        return 0;
    }

    signal(SIGINT, Stop); 

    printf("Server Port : %d\n", atoi(argv[1]));

    /*IPV4 , TCP, defult protocol*/
     fd = socket(AF_INET, SOCK_STREAM, 0);
    if ( fd < 0 )
    {
        perror("socket");
        exit(1);
    }
    ThreadPool_t *Pool = PoolInit(1 ,10, 100);

	if ( !Pool ) 
    {
		ERROR("threadpool_create false\n");
		return -1;
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
 //       threadpool_destroy(pool, FLAGS_WAIT_TASK_EXIT);
		return -1;
	}

	if ( listen(fd, 10) < 0 ) {
		perror("listen");
 //       threadpool_destroy(pool, FLAGS_WAIT_TASK_EXIT);
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
	    printf("client IP is: %s, client port is: %d, socket_fd addr = %p\n", buf, ntohs(client_addr.sin_port), &clientfd);

		 int rc = AddTask(Pool, test, (void *)&clientfd);
		 if (rc < 0) {
            // threadpool_destroy(pool, FLAGS_WAIT_TASK_EXIT);
			ERROR("threadpool_create false");
			return -1;
		}

    }

	// threadpool_destroy(pool, FLAGS_WAIT_TASK_EXIT);

    return 0;
}