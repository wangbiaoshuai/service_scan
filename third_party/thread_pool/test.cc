#include "thread_pool.h"

#include <stdio.h>
#include <unistd.h>

void* process(void* args) //回调函数
{
    printf("threadid is %0X, working on task %d\n", pthread_self(), *(int *)args);
    sleep(1);
    return NULL;
}

int main()
{
    ThreadPool thread_pool;
    if(thread_pool.Init(5) < 0)
    {
        printf("thread pool init failed\n");
        return -1;
    }

    int work[50] = {0};
    for(int i = 0; i < 50; i++)
    {
        work[i] = i;
        thread_pool.AddJob(process, &(work[i]));
    }
    sleep(20);
    thread_pool.Destroy();
    return 0;
}
