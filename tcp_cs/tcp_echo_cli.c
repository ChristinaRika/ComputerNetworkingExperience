#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_CMD_STR 100

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("input");
        return 1;
    }
    //socket
    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;

    server.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &(server.sin_addr));
    server.sin_port = htons(atoi(argv[2]));

    //connect
    if (connect(connfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect");
        return 1;
    }
    printf("[cli] server[%s:%s] is connected!\n", argv[1], argv[2]);

    while (1)
    {
        char buf[1024];
        //input
        fgets(buf, sizeof(buf), stdin);
        if (strncmp(buf, "exit", 4) == 0)
        {
            close(connfd);
            printf("[cli] connfd is closed!\n");

            printf("[cli] client is exiting!\n");
            return 0;
        }
        int len = strnlen(buf, MAX_CMD_STR);
        buf[len - 1] = '\0'; //change '\n' to '\0'
        write(connfd, &len, sizeof(len));
        write(connfd, buf, len);
        //server reply
        read(connfd, &len, sizeof(len));
        read(connfd,buf,len);
        printf("[echo_rep] %s\n", buf);
    }
}