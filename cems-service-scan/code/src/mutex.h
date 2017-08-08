#ifndef MUTEX_H_
#define MUTEX_H_

#include <pthread.h>
class Mutex
{
public:
    Mutex();
    ~Mutex();

    bool Lock();
    bool Unlock();
private:
    pthread_mutex_t mutex_;
};
#endif // LOCK_H_
