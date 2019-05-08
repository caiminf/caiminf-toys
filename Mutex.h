#pragma once

#include <pthread.h>
#include "Condition.h"

class Mutex
{
	friend class Condition;
public:
  Mutex();
  ~Mutex();
  void lock();
  void unlock();

private:
  pthread_mutex_t m_mutex;
};

class MutexLockGuard
{
public:
  MutexLockGuard(Mutex& mutex);
  ~MutexLockGuard();

private:
  Mutex& m_mutex;
};
