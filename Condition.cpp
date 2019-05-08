#include "Condition.h"

Condition::Condition(Mutex mutex) : m_mutex(mutex)
{
	m_cond = PTHREAD_COND_INITIALIZER;
}

void Condition::wait()
{
	m_mutex.lock();
	pthread_cond_wait(&m_cond, &m_mutex.m_mutex);
}

void Condition::notify()
{
	pthread_cond_signal(&m_cond);
}

void Condition::notify_all()
{
	pthread_cond_broadcast(&m_cond);
}
