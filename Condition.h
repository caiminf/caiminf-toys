#pragma once

#include "Mutex.h"
#include <pthread.h>

class Condition
{
public:
	Condition(Mutex mutex);
	void wait();
	void notify();
	void notify_all();

private:
	Mutex& m_mutex;
	pthread_cond_t m_cond;
};
