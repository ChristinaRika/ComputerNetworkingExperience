#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#define MAX_CMD_STR 100

int sig_to_exit = 0;

void sig_int(int signo)
{
    if(signo == SIGINT){
        printf("[srv] SIGINT is coming!\n");
        sig_to_exit = 1;
    }
}
void setSigInt(void){
    struct sigaction act;
    act.sa_handler = sig_int;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);
}
int startup(const char *ip, int port)
{
    int srv_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (srv_socket < 0)
    {
        perror("socket\n");
        exit(-1);
    }
    int opt = 1;
    setsockopt(srv_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in local;
    local.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &(local.sin_addr));
    local.sin_port = htons(port);

    if (bind(srv_socket, (struct sockaddr *)&local, sizeof(local)) < 0)
    {
        perror("bind\n");
        exit(-2);
    }

    if (listen(srv_socket, 5) < 0)
    {
        perror("listen\n");
        exit(-1);
    }

    return srv_socket;
}

void main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("input");
        exit(EXIT_FAILURE);
    }
    printf("[srv] server[%s:%s] is initializing!\n", argv[1], argv[2]);

    int listenfd = startup(argv[1], atoi(argv[2])); //socket,bind,listen
    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    //sigint
    setSigInt();

    while (1)
    {
        int connfd = accept(listenfd, (struct sockaddr *)&client, &len);
        //printf("connfd: %d\n",connfd);//---debug---
        if (connfd == -1 && errno == EINTR)
        {
            close(listenfd);
            printf("[srv] listenfd is closed!\n");
            break;
        }
        else
        {
            //show the client connect message
            struct sockaddr_in sa;
            int len = sizeof(sa);
            if (!getpeername(connfd, (struct sockaddr *)&sa, &len))
            {
                char ipv4_addr[16];
                inet_ntop(AF_INET, &(sa.sin_addr), ipv4_addr, sizeof(ipv4_addr));
                printf("[srv] client[%s:%d] is accepted!\n", ipv4_addr, ntohs(sa.sin_port));
            }

            while (1)
            {
                char buf[1024];
                int rlen = 0;
                memset(buf,'\0',sizeof(buf));
                //read(connfd, &rlen, sizeof(rlen));
                if ((read(connfd, &rlen, sizeof(rlen)) == 0))
                {
                    close(connfd);
                    printf("[srv] connfd is closed!\n");
                    break;
                }
                read(connfd, buf, rlen);
                printf("[echo_rqt] %s\n", buf);
                rlen = strnlen(buf, MAX_CMD_STR);
                write(connfd,&rlen,sizeof(rlen));
                write(connfd, buf, rlen);//sent to client(main to solve)
            }
        }    
    }

    printf("[srv] server is exiting!\n");
    exit(EXIT_SUCCESS);
}