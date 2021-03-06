#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Log.h"

static void Usage(const char* proc)
{
    printf("%s : [server_port]", proc);
}

//./client server_ip  server_port
int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }

    //1.创建socket
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0)
    {
        perror("sock");
        return 1;
    }
    //2.connect
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* 自动选择可用IP */

    if( connect(sock,(struct sockaddr*)&server,sizeof(server)) < 0 )
    {
        perror("connect");
        close(sock);
        return 2;
    }
    
    char buf[1024];
    char send_buffer[100] = "hello server i am client!";
    int i = 0;
    //先写后读
    while(1)
    {
        for (; i < 200; ++i )
        {
            // send
            int num = write(sock, (char *)send_buffer, strlen(send_buffer));
            if (num <= 0) {
                printf("send error\n");
                return -1;
            }
        
            // recv
            num = recv(sock, buf, 1024, 0);
            if (num > 0) // success
            {
                printf("rcv buffer = %s,:%d\n", buf, i);
            }
        }

        // WARNING("please Enter# ");
        // fflush(stdout);
        // ssize_t s = read(0, buf, sizeof(buf) - 1);
        // if(s > 0)
        // {

        //     buf[s - 1] = 0;
        //     write(sock,buf,strlen(buf));
        //     ssize_t _s = read(sock,buf,sizeof(buf) - 1);
        //     if(_s > 0)
        //     {
        //         buf[_s] = 0;
        //         WARNING("server echo# %s",buf);
        //     }
        //     else
        //     {
        //         ERROR("Server exit");
        //         break;
        //     }
        // }
    }
    close(sock);

    return 0;
}
