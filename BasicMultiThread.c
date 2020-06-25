#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>

void *Loop_1(void);
void *Loop_2(void);

int N = 0;

void main()
{
    pthread_t thread_1, thread_2;

    if((thread_1 = pthread_create(&thread_1, NULL, Loop_1, NULL)) == -1)
    {
        printf("Thread Create Fail.\n");
        exit(1);
    }

    if((thread_2 = pthread_create(&thread_2, NULL, Loop_2, NULL)) == -1)
    {
        printf("Thread Create Fail.\n");
        exit(1);
    }

    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);

    getchar();
}

void* Loop_1(void)
{
    for(int i = 0; i < 100; i++)
    {
        N++;
        _sleep(1);
    }
}
void* Loop_2(void)
{
    for(int i = 0; i < 100; i++)
    {
        printf("N: %d\n", N);
        _sleep(1);
    }
}