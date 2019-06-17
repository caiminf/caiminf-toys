#pragma once
#include <sys/types.h>
#include <map>
#include <string>

#define int64_t long long

const int DEFAULT_PORT = 8000;
const int MAX_RECV_BUFFER_LEN = 4096;
const int MAX_SEND_BUFFER_LEN = 4096;
const int MAX_EVENTS = 256;
const int REQUEST_ID_LEN = 8;

const int MAGIC = 12345678;

struct TaskInfo
{
	std::string reqId;
	int64_t iterateTimes;
	uint64_t sendTime;
	int sock;
};

////////////////////////////////////////////////////////////
//����ϵͳ��ĳ��ʱ��㿪ʼ��ʱ�����
//nTypeΪʱ�䵥λ:
//   TICK_TYPE_SECOND      0   -- unit is second.        ��
//   TICK_TYPE_MILLISECOND 1   -- unit is millisecond. ����
//   TICK_TYPE_MICROSECOND 2   -- unit is microsecond. ΢��
int64_t GetTickCount(int nType = 1);

int SetNonBlocking(int fd);

std::map<std::string, std::string> FormStringToMap(std::string reqStr);
