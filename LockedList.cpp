#include "LockedList.h"

template <typename T>
bool LockedList<T>::pop(T& res)
{
	MutexLockGuard guard(m_mutex);
	if (empty())
	{
		return false;
	}
	res = m_queue.front();
	m_queue.pop();
	return true;
}

template <typename T>
void LockedList<T>::push(const T& element)
{
	MutexLockGuard guard(m_mutex);
	m_queue.push(element);
}

template <typename T>
bool LockedList<T>::empty()
{
	return size() == 0;
}

template <typename T>
int LockedList<T>::size()
{
	return m_queue.size();
}