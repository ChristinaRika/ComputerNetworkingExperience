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

#define bprintf(fp, format, ...)            \
    if (fp == NULL)                         \
    {                                       \
        printf(format, ##__VA_ARGS__);      \
    }                                       \
    else                                    \
    {                                       \
        printf(format, ##__VA_ARGS__);      \
        fprintf(fp, format, ##__VA_ARGS__); fflush(fp);\
    }

#define MAX_CMD_STR 100

int sig_to_exit = 0;
int sig_type = 0;
//server result
char p_result[20];
FILE *p_res;
//student result
char c_result[20];
FILE *fp_res;

void sig_int(int signo)
{
    sig_type = signo;
    bprintf(p_res, "[srv](%d) SIGINT is coming!\n", getpid());
    sig_to_exit = 1;
}
void sig_pipe(int signo)
{
    sig_type = signo;
    bprintf(p_res, "[srv](%d) SIGPIPE is coming!\n", getpid());
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
int echo_rep(int connfd)
{
    int PIN = 0;

    while (1)
    {
        //TO DO--------HANDLE PDU
        
        char data_buf[MAX_CMD_STR + 1] = {'\0'};
        char pdu[MAX_CMD_STR + 9] = {'\0'};
        //read(connfd, &rlen, sizeof(rlen));
        
        if (((read(connfd, pdu, 8)) == 0)) //client exit
        {
            close(connfd);
            bprintf(fp_res, "[srv](%d) connfd is closed!\n", getpid());
            break;
        }
        //get PIN;
        char PIN_str[5];
        strncpy(PIN_str, &(pdu[0]), 4);
        PIN_str[4] = '\0';
        PIN = atoi(PIN_str);
        //get LEN;
        char LEN_str[5];
        strncpy(LEN_str, &(pdu[4]), 4);
        LEN_str[4] = '\0';

        int i = 8;
        int pduLen = 8 + atoi(LEN_str);
        while(i < pduLen){
            int rlen = read(connfd, &pdu[i], pduLen - i);
            i += rlen;
        }
        //get data
        strncpy(data_buf, &pdu[8], atoi(LEN_str));
        data_buf[atoi(LEN_str)] = '\0';

        bprintf(fp_res, "[echo_rqt](%d) %s\n", getpid(), data_buf);
        i = 0;
        while(i < pduLen){
            int wlen = write(connfd, &pdu[i], pduLen - i);
            i += wlen;
        }
        //write(connfd, pdu, sizeof(pdu));//echo reply
    }
    return PIN;
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
    printf("[srv](%d) %s is opened!\n", getpid(), p_result);

    bprintf(p_res, "[srv](%d) server[%s:%s] is initializing!\n", getpid(), argv[1], argv[2]);

    int listenfd = startup(argv[1], atoi(argv[2])); //socket,bind,listen
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    //sigchld
    setSigChld();
    //sigint
    setSigInt();
    //sigpipe
    setSigPipe();

    while (!sig_to_exit)
    {
        int connfd = accept(listenfd, (struct sockaddr *)&client, &len);
        if (connfd == -1 && errno == EINTR)
        {
            close(listenfd);
            bprintf(p_res, "[srv](%d) listenfd is closed!\n", getpid());
            break;
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(-1);
        }
        else if (pid == 0) //child process.
        {
            sprintf(c_result, "stu_srv_res_%d.txt", getpid()); //to store result
            fp_res = fopen(c_result, "w");
            printf("[srv](%d) %s is opened!\n", getpid(), c_result);
            bprintf(fp_res, "[srv](%d) child process is created!\n", getpid());
            close(listenfd);
            bprintf(fp_res, "[srv](%d) listenfd is closed!\n", getpid());

            int PIN = echo_rep(connfd);
            printf("[srv](%d) %s is closed!\n", getpid(), p_result); //
            
            char fileName[20];
            sprintf(fileName, "stu_srv_res_%d.txt", PIN);
            
            rename(c_result, fileName);
            bprintf(fp_res, "[srv](%d) res file rename done.\n", getpid());

            bprintf(fp_res, "[srv](%d) child process is going to exit!\n", getpid());
            fclose(fp_res);
            exit(EXIT_SUCCESS);
        }
        else
        {   
            //parent
            struct sockaddr_in sa;
            int len = sizeof(sa);
            if (!getpeername(connfd, (struct sockaddr *)&sa, &len))
            {
                char ipv4_addr[16];
                inet_ntop(AF_INET, &(sa.sin_addr), ipv4_addr, sizeof(ipv4_addr));
                bprintf(p_res, "[srv](%d) client[%s:%d] is accepted!\n",getpid(), ipv4_addr, ntohs(sa.sin_port)); 
            }
            close(connfd);
        }
    }
    bprintf(p_res, "[srv](%d) parent process is going to exit!\n", getpid());

    fclose(p_res);
    printf("[srv](%d) %s is closed\n", getpid(), p_result);

    exit(EXIT_SUCCESS);
}