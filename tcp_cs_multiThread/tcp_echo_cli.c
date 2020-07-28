//TODO :MAKE MULTI THREAD
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>

#define bprintf(fp, format, ...)            \
    if (fp == NULL)                         \
    {                                       \
        printf(format, ##__VA_ARGS__);      \
    }                                       \
    else                                    \
    {                                       \
        printf(format, ##__VA_ARGS__);      \
        fprintf(fp, format, ##__VA_ARGS__); \
        fflush(fp);                         \
    }

#define MAX_CMD_STR 100
char result[20];
FILE *res;
int PIN;
FILE *rd;
struct connState
{
    int connfd;
};

void setSigChld(void)
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_NOCLDWAIT;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(0);
    }
}

void *echo_rep(void *arg)
{
    pthread_detach(pthread_self());
    bprintf(res, "[cli](%d) pthread_detach is done!\n", syscall(SYS_gettid));
    int connfd = ((struct connState *)arg)->connfd;
    while (1)
    {
        int len_h,len_n;
        int pin_h,pin_n;
        char data_buf[MAX_CMD_STR + 1];
        memset(data_buf, '\0', sizeof(data_buf));
        
        fgets(data_buf, MAX_CMD_STR, rd);
        len_h = strnlen(data_buf, MAX_CMD_STR);
        if ((strncmp(data_buf, "exit", 4) == 0) || (len_h == 0))
        {
            shutdown(connfd, SHUT_WR);
            bprintf(res, "[cli](%d) shutdown is called with SHUT_WR!\n", syscall(SYS_gettid));
            fclose(rd); //close read file

            bprintf(res, "[cli](%d) child thread is going to exit!\n", syscall(SYS_gettid));
            return NULL;
        }   
        data_buf[len_h - 1] = '\0'; //change '\n' to '\0'
        pin_h = PIN;
        pin_n = htonl(pin_h);
        len_n = htonl(len_h);

        write(connfd, &pin_n,sizeof(pin_n));
        write(connfd, &len_n,sizeof(len_n));
        write(connfd, data_buf, len_h);         //send pdu
    }
}
void str_cli(int connfd, pid_t ptid){
    struct connState arg;
    arg.connfd = connfd;
    pthread_t tid;
    pthread_create(&tid, NULL, echo_rep, (void *)&arg);

    while (1)
    {
        int len_h,len_n;
        int pin_h,pin_n;

        char data_buf[MAX_CMD_STR + 1];

        //server reply
        memset(data_buf, '\0', sizeof(data_buf));
        
        if(read(connfd, &pin_n,sizeof(pin_n)) <= 0){
            close(connfd);
            bprintf(res, "[cli](%d) connfd is closed!\n", ptid);
            bprintf(res, "[cli](%d) parent thread is going to exit!\n",ptid);
            break;
        }

        read(connfd, &len_n,sizeof(len_n));
        read(connfd, data_buf, ntohl(len_n));//read header

        bprintf(res, "[echo_rep](%d) %s\n", ptid, data_buf);
    }
    return;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        perror("input");
        return 1;
    }
    int ptid = syscall(SYS_gettid);
    PIN = atoi(argv[3]);

    sprintf(result, "stu_cli_res_%d.txt", PIN); //to store result, tiis is parent.
    res = fopen(result, "w");
    printf("[cli](%d) %s is created!\n", ptid, result);

    memset(result, '\0', sizeof(result));
    sprintf(result, "stu_cli_res_%d.txt", PIN); //to store result
    res = fopen(result, "w");
    printf("[cli](%d) %s is created!\n", ptid, result);

    char toRead[10];
    sprintf(toRead, "td%d.txt", PIN); //to read test data
    rd = fopen(toRead, "r");

    //socket
    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;

    server.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &(server.sin_addr));
    server.sin_port = htons(atoi(argv[2]));
    //sigchld
    setSigChld();

    //connect
    if (connect(connfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect");
        return 1;
    }
    bprintf(res, "[cli](%d) server[%s:%s] is connected!\n", ptid, argv[1], argv[2]);

    str_cli(connfd, ptid);//read/write func

    fclose(res); //close result file
    printf("[cli](%d) %s is closed!\n", ptid, result);

    return 0;
}