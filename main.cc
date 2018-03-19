#include <stdio.h>
#include <stdlib.h>

#include "components/Thread.h"

void *thread_body(void *arg)
{
    printf("abc\n");
    return NULL;
}

int main()
{
    for (int i = 0; i < 10; ++i)
    {
        Thread thread(thread_body);
    }
    return 0;
}