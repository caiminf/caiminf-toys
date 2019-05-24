#include "Util.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

using std::string;
using std::map;

////////////////////////////////////////////////////////////
//返回系统从某个时间点开始的时间计数
//nType为时间单位:
//   TICK_TYPE_SECOND      0   -- unit is second.        秒
//   TICK_TYPE_MILLISECOND 1   -- unit is millisecond. 毫秒
//   TICK_TYPE_MICROSECOND 2   -- unit is microsecond. 微秒
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

int SetNonBlocking(int fd) {
	int s;
	s = fcntl(fd, F_GETFL, 0);
	s |= O_NONBLOCK;
	return fcntl(fd, F_SETFL, s);
}

map<string, string> FormStringToMap(string reqStr)
{
	map<string, string> res;
	reqStr += '&';
	int lastPos = 0;
	for (int andPos = 0; andPos < reqStr.length(); andPos++)
	{
		if (reqStr[andPos] != '&')
		{
			continue;
		}
		int ePos = lastPos;
		while (reqStr[ePos] != '=' & ePos < andPos)
		{
			ePos++;
		}
		string key = reqStr.substr(lastPos, ePos - lastPos);
		string value = reqStr.substr(ePos + 1, andPos - ePos - 1);
		res[key] = value;
		lastPos = andPos + 1;
	}
	return res;
}
