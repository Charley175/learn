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


static void Usage(const char* proc)
{
    printf("%s : [server_ip][server_port]",proc);
}

//./client server_ip  server_port
int main(int argc,char* argv[])
{
    if(argc != 3)
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
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);//将点分式的字符串转换为能在网络上传输的

    if(connect(sock,(struct sockaddr*)&server,sizeof(server)) < 0)
    {
        perror("connect");
        return 2;
    }
    char buf[1024];
    //先写后读
    while(1)
    {
        printf("please Enter# ");
        fflush(stdout);
        ssize_t s = read(0,buf,sizeof(buf) - 1);
        if(s > 0)
        {

            buf[s - 1] = 0;
            write(sock,buf,strlen(buf));
            ssize_t _s = read(sock,buf,sizeof(buf) - 1);
            if(_s > 0)
            {
                buf[_s] = 0;
                printf("server echo# %s\n",buf);
            }
        }
    }
    close(sock);
    return 0;
}
