#include "Semaphore.h"

Semaphore::Semaphore()
{
	sem_init(&m_sem, 0, 0);
}

void Semaphore::Wait()
{
	sem_wait(&m_sem);
}

void Semaphore::Post()
{
	sem_post(&m_sem);
}