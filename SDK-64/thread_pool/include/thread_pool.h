#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <list>
#include <pthread.h>

typedef struct Job
{
    void* (*callback_function) (void* args);
    void * args;

    Job() {callback_function = NULL; args = NULL;}
} Job;

class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();
    int Init(int thread_num);
    void AddJob(void* (*callback_function)(void* args), void* args);
    void thread_routine();
    void Destroy();
private:
    int thread_num_;
    pthread_t* threads_;
    pthread_mutex_t mutex_;
    pthread_cond_t cond_;
    std::list<Job> job_list_;
    bool shutdown_;
};
#endif // _THREAD_POOL_H
