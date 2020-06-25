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

#define bprintf(fp, format, ...)            \
    if (fp == NULL)                         \
    {                                       \
        printf(format, ##__VA_ARGS__);      \
    }                                       \
    else                                    \
    {                                       \
        printf(format, ##__VA_ARGS__);      \
        fprintf(fp, format, ##__VA_ARGS__);fflush(fp); \
    }

#define MAX_CMD_STR 100
/*
struct HEAD
{
    int PIN;
    int LEN;
};
struct PDU
{
    struct HEAD header;
    char data[MAX_CMD_STR + 1];
}; //PDU
*/
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

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        perror("input");
        return 1;
    }

    int PIN = 0;
    pid_t pid;
    int i;
    int childProcessNum = atoi(argv[3]) - 1;
    char result[20];
    FILE *res;

    sprintf(result, "stu_cli_res_%d.txt", PIN); //to store result, tiis is parent.
    res = fopen(result, "w");
    printf("[cli](%d) %s is created!\n", getpid(), result);

    for (i = 0; i < childProcessNum; i++)
    {
        pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(-1);
        }
        else if (pid == 0)
        { //child
            PIN = i + 1;

            memset(result, '\0', sizeof(result));
            sprintf(result, "stu_cli_res_%d.txt", PIN); //to store result
            res = fopen(result, "w");
            printf("[cli](%d) %s is created!\n", getpid(), result);

            bprintf(res, "[cli](%d) child process %d is created!\n", getpid(), PIN);
            break;
        }
    }

    char toRead[10];
    sprintf(toRead, "td%d.txt", PIN); //to read test data
    FILE *rd = fopen(toRead, "r");

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
    bprintf(res, "[cli](%d) server[%s:%s] is connected!\n", getpid(), argv[1], argv[2]);
    
    while (1)
    {
        //TO MAKE PDU [PIN + LEN + DATA]//4 + 4 + (MAX_CMD_STR + 1)
        //TO DO...
        char PIN_str[5];
        char LEN_str[5];
        char data_buf[MAX_CMD_STR + 1];
        char pdu[MAX_CMD_STR + 9];
        memset(pdu,'\0',sizeof(pdu));
        memset(data_buf,'\0',sizeof(data_buf));
        //input
        fgets(data_buf, MAX_CMD_STR, rd);
        if (strncmp(data_buf, "exit", 4) == 0)
        {
            fclose(rd); //close read file
            close(connfd);
            bprintf(res, "[cli](%d) connfd is closed!\n", getpid());

            bprintf(res, "[cli](%d) process is going to exit!\n", getpid());
            fclose(res); //close result file
            printf("[cli](%d) %s is closed!\n",getpid(), result);
            return 0;
        }
        int len = strnlen(data_buf, MAX_CMD_STR);
        data_buf[len - 1] = '\0'; //change '\n' to '\0'
        sprintf(PIN_str,"%04d",PIN);
        sprintf(LEN_str,"%04d",len);
        //TO STORE PDU
        sprintf(pdu, "%s%s%s",PIN_str, LEN_str, data_buf);
        
        int pduLen = 8 + len;
        int i = 0;
        while(i < pduLen){
            int wlen = write(connfd, &pdu[i], pduLen - i);
            i += wlen;
        }
        
        //write(connfd, pdu, sizeof(pdu));         //send pdu
        //server reply
        memset(data_buf,'\0',sizeof(data_buf));
        memset(LEN_str,'\0',sizeof(LEN_str));
        memset(pdu,'\0',sizeof(pdu));        
        i = 0;
        i += read(connfd, &pdu[0], 8);
        strncpy(LEN_str, &(pdu[4]), 4);
        LEN_str[4] = '\0';
        pduLen = i + atoi(LEN_str);
        while(i < pduLen){
            int rlen = read(connfd, &pdu[i], pduLen - i);
            i += rlen;
        }
        //read(connfd, &pdu[i], atoi(LEN_str));

        strncpy(data_buf, &pdu[8], atoi(LEN_str));
        data_buf[atoi(LEN_str)] = '\0';

        bprintf(res, "[echo_rep](%d) %s\n", getpid(), data_buf);
    }
}