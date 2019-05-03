#include "Thread.h"

Thread::Thread(ThreadBody_t thread_body) : m_tid()
{
    pthread_create(&m_tid, NULL, thread_body, NULL);
}

Thread::~Thread()
{
    pthread_join(m_tid, NULL);
}