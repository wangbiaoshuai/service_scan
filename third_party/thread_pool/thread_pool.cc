#include "thread_pool.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

ThreadPool::ThreadPool() :
thread_num_(0),
threads_(NULL),
shutdown_(false)
{
}

ThreadPool::~ThreadPool()
{
}

void ThreadPool::thread_routine()
{
    printf("start thread %0X\n", pthread_self());
    while(1)
    {
        pthread_mutex_lock(&mutex_);
        while(job_list_.empty() && !shutdown_)
        {
            printf("thread %0X is waiting\n", pthread_self());
            pthread_cond_wait(&cond_, &mutex_);
        }

        if(shutdown_)
        {
            pthread_mutex_unlock(&mutex_);
            printf("thread %0X will exit\n", pthread_self());
            return;
        }

        printf("thread %0X is working\n", pthread_self());
        Job tmpj;
        if(!job_list_.empty())   //防止惊群效应
        {
            tmpj = job_list_.front();
            job_list_.pop_front();
        }
        pthread_mutex_unlock(&mutex_);

        if(tmpj.callback_function != NULL)
        {
            (*(tmpj.callback_function))(tmpj.args);
        }
    }

    return;
}

void* thread_function(void* args)
{
    ThreadPool* context = (ThreadPool*)args;
    context->thread_routine();
    pthread_exit(NULL);
}

int ThreadPool::Init(int thread_num)
{
    if(thread_num <= 0)
        return -1;

    thread_num_ = thread_num;
    threads_ = new pthread_t[thread_num_];
    pthread_mutex_init(&mutex_, NULL);
    pthread_cond_init(&cond_, NULL);
    shutdown_ = false;

    int i;
    for(i = 0; i < thread_num_; i++)
    {
        int res = pthread_create(&(threads_[i]), NULL, thread_function, this);
        if(res < 0)
        {
            printf("create thread error:%s\n", strerror(errno));
            break;
        }
    }

    if(i != thread_num_)
    {
        return -1;
    }

    return 0;
}

void ThreadPool::AddJob(void* (*callback_function)(void* args), void* args)
{
    Job tmpj;
    tmpj.callback_function = callback_function;
    tmpj.args = args;

    pthread_mutex_lock(&mutex_);
    job_list_.push_back(tmpj);
    pthread_cond_signal(&cond_);
    pthread_mutex_unlock(&mutex_);
}

void ThreadPool::Destroy()
{
    if(shutdown_ == true)
        return;

    shutdown_ = true;
    pthread_cond_broadcast(&cond_);

    for(int i = 0; i < thread_num_; i++)
    {
        pthread_join((threads_[i]), NULL);
    }

    pthread_cond_destroy(&cond_);
    pthread_mutex_destroy(&mutex_);

    if(threads_ != NULL)
    {
        delete[] threads_;
        threads_ = NULL;
    }

    if(!job_list_.empty())
    {
        printf("%d jobs not been execute.\n", job_list_.size());
        job_list_.clear();
    }

    return;
}

