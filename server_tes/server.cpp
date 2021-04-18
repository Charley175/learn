#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>

static void Usage(const char* proc)
{
    printf("%s : [server_ip][server_port]\n",proc);
}

int  startup(char* ip,int  port)
{
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0)
    {
        perror("sock");
        exit(1);
    }
    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = inet_addr(ip);
    socklen_t len = sizeof(local);
    if(bind(sock,(struct sockaddr*)&local,len)< 0)
    {
        perror("bind");
        exit(2);
    }

    if(listen(sock,3) < 0)
    {
        perror("listen");
        exit(3);
    }
    return sock;
}

int main(int argc,char* argv[])
{
    if(argc != 3)
    {
        Usage(argv[0]);
        exit(4);
    }
        //接待客人的人
    int listen_sock = startup(argv[1],atoi(argv[2]));
    printf("sock: %d\n",listen_sock);
    //需要让子进程的子进程去提供服务
    //父进程继续监听

    char buf[1024];
    while(1)
    {
        //接收client套接字的信息
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        //newsock == 服务人员
        int newsock = accept(listen_sock,(struct sockaddr*)&client,&len);
        if(newsock < 0)
        {
            perror("accept");
            continue;//继续接听下一个
        }
        //将网络中的数据转换为主机用户可以看懂的数据
        printf("get new client [%s: %d]\n",\
              inet_ntoa(client.sin_addr.s_addr,\
                       ntohs(client.sin_port)));

    pid_t id = fork();
    if(id < 0)
    {
        perror("fork");
        close(newsock);
        return 1;
    }
    else if(id == 0)
    {
        //因为子进程会继承父进程的文件描述符表，而子进程只需要newsock（提供服务的套接字）
        //child
        close(listen_sock);//子进程关闭监听套接字
        if(fork() > 0)
        {
            //child ---->father
            exit(0);//子进程充当父进程的角色，父进程退出后，会导致子进程成为孤儿进程
        }
        //child->child-->也就是孙子进程（但是注意linux中只有父子关系，没有所谓的爷孙关系）
        //让孙子进程去服务--？》读和写
        while(1)
        {
             int s = read(newsock,buf,sizeof(buf) - 1);
            if(s > 0)
            {
                buf[s] = '\0';
                printf("client# %s\n",buf);
                write(newsock,buf,strlen(buf));//服务器将读到的信息给客户端回显回去
                }else if(s == 0)
             {
                printf("client quit\n");
                break;
             }
            else{
                break;
             }
        }
        close(newsock);
        exit(1);//当子进程执行完之后需要退出
    }
     else{
        //father
         close(newsock);//父进程只负责监听
         waitpid(id,NULL,0);
    }
        //1.read   2.write
    }
    close(listen_sock);
    return 0;
}