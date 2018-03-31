#include <pthread.h>

class MutexLock
{
public:
  MutexLock();
  ~MutexLock();
  void lock();
  void unlock();

private:
  pthread_mutex_t mutex_;
};

class MutexLockGuard
{
public:
  MutexLockGuard(MutexLock& mutex);
  ~MutexLockGuard();

private:
  MutexLock& mutex_;
};