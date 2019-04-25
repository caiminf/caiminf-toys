#include "Util.h"
#include <stdlib.h>
#include <time.h>

////////////////////////////////////////////////////////////
//����ϵͳ��ĳ��ʱ��㿪ʼ��ʱ�����
//nTypeΪʱ�䵥λ:
//   TICK_TYPE_SECOND      0   -- unit is second.        ��
//   TICK_TYPE_MILLISECOND 1   -- unit is millisecond. ����
//   TICK_TYPE_MICROSECOND 2   -- unit is microsecond. ΢��
int64_t GetTickCount(int nType/* = 1*/)
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	if (0 == nType)
	{
		return ((int64_t)tp.tv_sec);
	}
	if (1 == nType)
	{
		return ((int64_t)tp.tv_sec) * 1000 + ((int64_t)tp.tv_nsec) / 1000000;
	}
	if (2 == nType)
	{
		return ((int64_t)tp.tv_sec) * 1000000 + ((int64_t)tp.tv_nsec) / 1000;
	}
	return 0;
}