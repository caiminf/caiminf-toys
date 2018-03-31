#include "MutexLock.h"

MutexLock::MutexLock()
{
    pthread_mutex_init(&mutex_, NULL);
}

MutexLock::~MutexLock()
{
    pthread_mutex_destroy(&mutex_);
}

void MutexLock::lock()
{
    pthread_mutex_lock(&mutex_);
}

void MutexLock::unlock()
{
    pthread_mutex_unlock(&mutex_);
}

MutexLockGuard::MutexLockGuard(MutexLock& mutex) : mutex_(mutex)
{
    mutex_.lock();
}

MutexLockGuard::~MutexLockGuard()
{
    mutex_.unlock();
}