#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>

typedef void *ThreadBody_t(void *);

class Thread
{
  public:
    Thread(ThreadBody_t);
    virtual ~Thread();

  private:
    pthread_t m_tid;
};

#endif