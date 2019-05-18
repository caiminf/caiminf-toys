#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string>
#include <map>
#include <queue>
#include <sys/epoll.h>

#include "Util.h"
#include "LockedList.h"

using std::map;
using std::string;
using std::queue;

struct TaskInfo
{
	string reqId;
	int64_t iterateTimes;
	uint64_t recvTime;
	int sock;
};

LockedList<TaskInfo> g_TaskList;
bool g_exitFlag;

int CreateTcpSocket(unsigned short port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		perror("cannot create socket");
		return -1;
	}
	const int on = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0) {
		perror("setsockopt failed");
		return -2;
	}

	struct sockaddr_in addr;
	memset((void *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	if (0 != bind(fd, (struct sockaddr *)&addr, sizeof(addr)))
	{
		perror("bind failed");
		return -3;
	}

	if (0 != listen(fd, 1024))
	{
		perror("listen failed");
		return -4;
	}
	return fd;
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

int ParseReqToTask(const char*inputBuf, int inputBufLen, TaskInfo* resTask, int sock)
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
		printf("ierate_times not found\n");
		return -3;
	}
	else
	{
		resTask->iterateTimes = atoll(iter->second.c_str());
	}
	resTask->sock = sock;
	resTask->recvTime = GetTickCount(1);
	return 0;
}

void ReadTaskFromSocket(int fd)
{
	bool done = false;
	while (true)
	{
		char recvBuf[MAX_RECV_BUFFER_LEN] = { 0 };
		
		int count = read(fd, recvBuf, sizeof(recvBuf));
		if (count == 0)
		{
			// EOF
			done = true;
			break;
		}
		else if (count == -1)
		{
			// error or EAGAIN
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
				perror("read error");
				done = true;
			}
			break;
		}
		else
		{
			TaskInfo taskInfo;
			int ret = ParseReqToTask(recvBuf, count, &taskInfo, fd);
			if (ret != 0)
			{
				printf("ParseReqToTask error, ret=%d\n", ret);
				continue;
			}
			g_TaskList.push(taskInfo);
		}
	}
}

int ProcessTask(const TaskInfo& task, string &response)
{
	int64_t tcStart = GetTickCount(1);
	int64_t iterateTimes = task.iterateTimes;
	const int pace = 100000000;
	int tmpPace = pace;
	for (int64_t i = 0; i < iterateTimes; i++)
	{
		if (--tmpPace == 0)
		{
			int64_t tc = GetTickCount(1);
			printf("num_remain = %ld, already cost %ldms\n", iterateTimes - i, tc - tcStart);
			tmpPace = pace;
		}
	}
	int64_t tcEnd = GetTickCount(1);
	int64_t cost = tcEnd - tcStart;
	response = "request_id=" + task.reqId + "costTime=" + std::to_string(cost);
	return 0;
}

void *WorkerThreadProc(void *lp)
{
	while (!g_exitFlag)
	{
		TaskInfo task = g_TaskList.pop();
		string response;
		int ret = ProcessTask(task, response);
		if (ret != 0)
		{
			printf("ProcessTask failed reqest_id=%s\n", task.reqId.c_str());
			continue;
		}
		int nbytes = write(task.sock, response.c_str(), response.length());
	}
}

int main()
{
	g_exitFlag = false;
	int listenFd = CreateTcpSocket(DEFAULT_PORT);
	if (listenFd < 0)
	{
		printf("create socket failed ret=%d\n", listenFd);
		return -1;
	}

	pthread_t tid;
	int ret = pthread_create(&tid, NULL, WorkerThreadProc, NULL);
	if (0 != ret)
	{
		perror("create connect thread fail");
		return -2;
	}

	struct epoll_event ev, events[MAX_EVENTS];
	int epollFd = epoll_create1(0);

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = listenFd;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, listenFd, &ev);

	while (!g_exitFlag)
	{
		int activeFdCnt = epoll_wait(epollFd, events, MAX_EVENTS, -1);
		for (int i = 0; i < activeFdCnt; i++)
		{
			if (events[i].data.fd == listenFd)
			{
				struct sockaddr_in clientAddr;
				socklen_t addrLen = sizeof(clientAddr);
				int connectSocket = accept(listenFd, (struct sockaddr *)&clientAddr, &addrLen);
				if (connectSocket == -1)
				{
					if (errno != EAGAIN && errno != EWOULDBLOCK)
					{
						perror("accept error");
					}
					continue;
				}
				SetNonBlocking(connectSocket);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = connectSocket;
				epoll_ctl(epollFd, EPOLL_CTL_ADD, connectSocket, &ev);
			}
			else
			{
				ReadTaskFromSocket(events[i].data.fd);
			}
		}
	}

	return 0;
}
