#pragma once

#include <queue>
#include "Condition.h"

template <typename T>
class LockedList
{
public:
	LockedList();
	T pop();
	void push(const T& element);
	bool empty();
	int size();

private:
	Condition m_cond;
	Mutex m_mutex;
	std::queue<T> m_queue;
};

template <typename T>
LockedList<T>::LockedList() : m_mutex()
                            , m_cond(m_mutex)
							, m_queue()
{
}

template <typename T>
T LockedList<T>::pop()
{
	MutexLockGuard guard(m_mutex);
	while (m_queue.empty())
	{
		m_cond.wait();
	}
	T front = m_queue.front();
	m_queue.pop();
	return T;
}

template <typename T>
void LockedList<T>::push(const T& element)
{
	{
		MutexLockGuard guard(m_mutex);
		m_queue.push(element);
	}
	m_cond.notify();
}

template <typename T>
bool LockedList<T>::empty()
{
	MutexLockGuard guard(m_mutex);
	return m_queue.empty();
}

template <typename T>
int LockedList<T>::size()
{
	MutexLockGuard guard(m_mutex);
	return m_queue.size();
}
