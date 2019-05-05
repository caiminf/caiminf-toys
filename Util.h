#pragma once
#include <sys/types.h>

#define int64_t long long

const int DEFAULT_PORT = 8000;
const int MAX_RECV_BUFFER_LEN = 4096;
const int MAX_SEND_BUFFER_LEN = 4096;

////////////////////////////////////////////////////////////
//����ϵͳ��ĳ��ʱ��㿪ʼ��ʱ�����
//nTypeΪʱ�䵥λ:
//   TICK_TYPE_SECOND      0   -- unit is second.        ��
//   TICK_TYPE_MILLISECOND 1   -- unit is millisecond. ����
//   TICK_TYPE_MICROSECOND 2   -- unit is microsecond. ΢��
int64_t GetTickCount(int nType = 1);