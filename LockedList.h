#pragma once

#include <queue>
#include "MutexLock.h"

template <typename T>
class LockedList
{
public:
	bool pop(T& res);
	void push(const T& elememt);
	bool empty();
	int size();

private:
	MutexLock m_mutex;
	std::queue<T> m_queue;
};