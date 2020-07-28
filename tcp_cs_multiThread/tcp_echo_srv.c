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

int sig_to_exit = 0;
int sig_type = 0;
//server result
char p_result[20];
FILE *p_res;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
struct clientState{
    int connfd;
};

void sig_int(int signo)
{
    sig_type = signo;
    bprintf(p_res, "[srv](%d) SIGINT is coming!\n", syscall(SYS_gettid));
    sig_to_exit = 1;
}
void sig_pipe(int signo)
{
    sig_type = signo;
    bprintf(p_res, "[srv](%d) SIGPIPE is coming!\n", syscall(SYS_gettid));
}
void setSigInt(void)
{
    struct sigaction act;
    act.sa_handler = sig_int;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0);
}
void setSigPipe(void)
{
    struct sigaction sigac_pip, old_sigac_pip;
    sigac_pip.sa_handler = sig_pipe;
    sigemptyset(&sigac_pip.sa_mask);
    sigac_pip.sa_flags = 0;
    sigac_pip.sa_flags |= SA_RESTART;
    sigaction(SIGPIPE, &sigac_pip, &old_sigac_pip);
}
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

    if (listen(srv_socket, 10) < 0)
    {
        perror("listen\n");
        exit(-1);
    }

    return srv_socket;
}
int echo_rep(FILE* fp_res, int connfd)
{
    int PIN = 0;

    while (1)
    {
        int len_h,len_n;
        int pin_h,pin_n;
        char data_buf[MAX_CMD_STR + 1] = {'\0'};

        if (((read(connfd, &pin_n, 4)) == 0)) //client exit
        {
            close(connfd);
            bprintf(fp_res, "[srv](%d) connfd is closed!\n", syscall(SYS_gettid));
            break;
        }
        //get PIN;
        PIN=ntohl(pin_n);
        //get LEN;
        read(connfd, &len_n, 4);
        len_h = ntohl(len_n);
        //get data
        read(connfd, data_buf, len_h);

        bprintf(fp_res, "[echo_rqt](%d) %s\n", syscall(SYS_gettid), data_buf);
        //echo reply
        write(connfd, &pin_n,sizeof(pin_n));
        write(connfd, &len_n,sizeof(len_n));
        write(connfd, data_buf, len_h); 
    }
    
    return PIN;
}

void *thread_handle(void *arg)
{
    pthread_detach(pthread_self());
    int tid = syscall(SYS_gettid);
    //student result
    char c_result[20];
    FILE *fp_res;
    sprintf(c_result, "stu_srv_res_%d.txt", tid); //to store result
    fp_res = fopen(c_result, "w");
    printf("[srv](%d) %s is opened!\n", tid, c_result);
    bprintf(fp_res, "[srv](%d) child thread is created!\n", tid);

    int connfd = ((struct clientState *)arg)->connfd;
    int PIN = echo_rep(fp_res, connfd);
    
    printf("[srv](%d) %s is closed!\n", tid, p_result); //
    char fileName[20];
    sprintf(fileName, "stu_srv_res_%d.txt", PIN);
    rename(c_result, fileName);
    bprintf(fp_res, "[srv](%d) res file rename done.\n", tid);

    bprintf(fp_res, "[srv](%d) child thread is going to exit!\n", tid);
    fclose(fp_res);   
}

void main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("input");
        exit(EXIT_FAILURE);
    }

    sprintf(p_result, "stu_srv_res_p.txt"); //to store result
    p_res = fopen(p_result, "w");
    printf("[srv](%d) %s is opened!\n", syscall(SYS_gettid), p_result);

    bprintf(p_res, "[srv](%d) server[%s:%s] is initializing!\n", syscall(SYS_gettid), argv[1], argv[2]);

    int listenfd = startup(argv[1], atoi(argv[2])); //socket,bind,listen
    struct sockaddr_in client;
    //sigchld
    setSigChld();
    //sigint
    setSigInt();
    //sigpipe
    setSigPipe();

    while (!sig_to_exit)
    {
        socklen_t sok_len = sizeof(client);
        int connfd = accept(listenfd, (struct sockaddr *)&client, &sok_len);
        if (connfd == -1 && errno == EINTR)
        {
            if (sig_type == SIGINT)
            {
                close(listenfd);
                bprintf(p_res, "[srv](%d) listenfd is closed!\n", syscall(SYS_gettid));
                bprintf(p_res, "[srv](%d) parent thread is going to exit!\n", syscall(SYS_gettid));

                fclose(p_res);
                printf("[srv](%d) %s is closed\n", syscall(SYS_gettid), p_result);

                exit(EXIT_SUCCESS);
            }
            continue;
        }
        //client state
        struct sockaddr_in sa;
        int len = sizeof(sa);
        if (!getpeername(connfd, (struct sockaddr *)&sa, &len))
        {
            char ipv4_addr[16];
            inet_ntop(AF_INET, &(sa.sin_addr), ipv4_addr, sizeof(ipv4_addr));
            bprintf(p_res, "[srv](%d) client[%s:%d] is accepted!\n", syscall(SYS_gettid), ipv4_addr, ntohs(sa.sin_port));
        }

        struct clientState cs;
        cs.connfd = connfd;
        pthread_t tid;
        pthread_create(&tid,NULL,thread_handle,(void *)&cs);
    }
}