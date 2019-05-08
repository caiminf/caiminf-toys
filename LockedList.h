#pragma once

#include <queue>
#include "Condition.h"

template <typename T>
class LockedList
{
public:
	LockedList();
	bool pop(T& res);
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
{
}

template <typename T>
bool LockedList<T>::pop(T& res)
{
	MutexLockGuard guard(m_mutex);
	m_cond.wait();
	res = m_queue.front();
	m_queue.pop();
	return true;
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
	return m_queue.empty();
}

template <typename T>
int LockedList<T>::size()
{
	return m_queue.size();
}
