#include <stdio.h>
#include <stdlib.h>

#include "components/Thread.h"
#include "components/MutexLock.h"

long g_sum = 0;
MutexLock g_mutex;

void *thread_body1(void *arg)
{
    MutexLockGuard lockGuard(g_mutex);
    for (int i = 0; i < 1000; ++i)
    {
        g_sum++;
    }
    return NULL;
}

void *thread_body2(void *arg)
{
    MutexLockGuard lockGuard(g_mutex);
    for (int i = 0; i < 100; ++i)
    {
        g_sum++;
    }
    return NULL;
}

int main()
{
    Thread thread1(thread_body1);
    Thread thread2(thread_body2);

    printf("g_sum = %d\n", g_sum);
    return 0;
}