#include "Connection.h"
#include <unistd.h>
#include <map>
#include <string>

using std::string;
using std::map;

Connection::Connection():m_bufEndPointer(0)
{
	memset(m_recvBuf, 0, sizeof(m_recvBuf));
}

int Connection::read(std::vector<TaskInfo>& tasks)
{
	while (true)
	{
		int count = ::read(	m_socket,
							m_recvBuf + m_bufEndPointer,
							sizeof(m_recvBuf) - m_bufEndPointer);
		if (count == 0)
		{
			// EOF
			break;
		}
		else if (count == -1)
		{
			// error or EAGAIN
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
				perror("read error");
			}
			break;
		}
		else
		{
			m_bufEndPointer += count;
		}
	}
	// all incoming bytes read

	do {
		int *magic = (int*)(m_recvBuf);
		
	
		if (*magic != MAGIC) // check magic failed
		{
			printf("bad magic: %d\n", *magic);
			memset(m_recvBuf, 0, sizeof(m_recvBuf));
			m_bufEndPointer = 0;
			break;
		}

		int *len = (int*)(&m_recvBuf[4]);
		if (*len + 2 > m_bufEndPointer) // not enough bytes read
		{
			break;
		}

		char msgBuf[MAX_RECV_BUFFER_LEN] = { 0 };
		memcpy(msgBuf, m_recvBuf + 2, *len);

		TaskInfo taskInfo;
		int ret = ParseReqToTask(msgBuf, &taskInfo);
		if (ret != 0)
		{
			printf("ParseReqToTask error, ret=%d\n", ret);
			memset(m_recvBuf, 0, sizeof(m_recvBuf));
			m_bufEndPointer = 0;
			break;
		}

		tasks.push_back(taskInfo);

		memcpy(m_recvBuf, m_recvBuf + 2 + *len, sizeof(m_recvBuf) - 2 - *len);
		m_bufEndPointer = m_bufEndPointer - 2 - *len;
	} while (m_bufEndPointer > 0);

	return tasks.size();
}


int Connection::ParseReqToTask(const char*inputBuf, TaskInfo* resTask)
{
	if (resTask == NULL)
	{
		printf("resTask is NULL\n");
		return -1;
	}
	map<string, string> reqMap = FormStringToMap(string(inputBuf));
	map<string, string>::iterator iter = reqMap.find("request_id");
	if (iter == reqMap.end())
	{
		printf("request_id not found\n");
		return -2;
	}
	else
	{
		resTask->reqId = iter->second;
	}
	iter = reqMap.find("iterate_times");
	if (iter == reqMap.end())
	{
		printf("iterate_times not found\n");
		return -3;
	}
	else
	{
		resTask->iterateTimes = atoll(iter->second.c_str());
	}
	iter = reqMap.find("send_time");
	if (iter == reqMap.end())
	{
		printf("send_time not found\n");
		return -3;
	}
	else
	{
		resTask->sendTime = atoll(iter->second.c_str());
	}
	resTask->sock = m_socket;
	return 0;
}