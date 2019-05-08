#pragma once

#include <semaphore.h>

class Semaphore
{
public:
	Semaphore();
	void Wait();
	void Post();
private:
	sem_t m_sem;
};