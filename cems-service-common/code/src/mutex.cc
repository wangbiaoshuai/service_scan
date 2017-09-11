#include "mutex.h"

Mutex::Mutex()
{
    pthread_mutex_init(&mutex_, NULL);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&mutex_);
}

bool Mutex::Lock()
{
    if(pthread_mutex_lock(&mutex_) != 0)
    {
        return false;
    }
    return true;
}

bool Mutex::Unlock()
{
    if(pthread_mutex_unlock(&mutex_) != 0)
    {
        return false;
    }
    return true;
}
