#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <map>
#include <string>
#include <vector>
#include <time.h>

#include "Util.h"

using std::string;
using std::map;
using std::vector;

static bool g_exitFlag;
static int64_t g_rangeStart;
static int64_t g_rangeStop;
static int g_step;
static int g_speed;
static int g_reqId;

string FormRequestString(map<string, string>& requestMap)
{
	string reqStr = "";
	for (map<string, string>::iterator iter = requestMap.begin(); iter != requestMap.end(); iter++)
	{
		reqStr += (iter->first + "=" + iter->second + "&");
	}
	reqStr.pop_back();
	return reqStr;
}

int ConnectToServer(const char *host, int port)
{
	int sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockFd < 0)
	{
		perror("create socket error");
		return -1;
	}

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	int ret = inet_pton(AF_INET, host, &servaddr.sin_addr);
	if (ret <= 0)
	{
		perror("inet_pton failed");
		return -2;
	}

	ret = connect(sockFd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	if (ret < 0)
	{
		perror("connect failed");
		return -3;
	}
	return sockFd;
}

void RandomVectorGen(int64_t start, int64_t stop, int cnt, vector<int64_t>& vec)
{
	vec.clear();
	int diff = stop - start;
	for (int i = 0; i < cnt; i++)
	{
		int r = rand() % diff;
		int64_t num = start + r;
		vec.push_back(num);
	}
}

string GetReqId()
{
	int reqId = __sync_fetch_and_add(&g_reqId, 1);
	if (reqId == 99999999)
	{
		g_reqId = 0;
	}
	string strReqId = std::to_string(reqId);
	int len = strReqId.length();
	for (int i = 0; i < REQUEST_ID_LEN - len; i++)
	{
		strReqId = "0" + strReqId;
	}
	return strReqId;
}

void *SendThreadProc(void *lp)
{
	int fd = *(int *)lp;
	vector<int64_t> calNum;
	RandomVectorGen(g_rangeStart, g_rangeStop, g_step, calNum);
	int i = 0;
	while (!g_exitFlag)
	{
		char sendBuf[MAX_SEND_BUFFER_LEN] = { 0 };
		string reqId = GetReqId();
		map<string, string> reqMap = {
			{ "request_id", reqId },
			{ "iterate_times", std::to_string(calNum[i]) },
			{ "send_time", std::to_string(GetTickCount(1))}
		};
		string reqStr = FormRequestString(reqMap);
		strncpy(sendBuf, reqStr.c_str(), MAX_SEND_BUFFER_LEN);

		printf("send msg to server: %s\n", sendBuf);
		int ret = send(fd, sendBuf, strnlen(sendBuf, MAX_SEND_BUFFER_LEN), 0);
		if (ret < 0)
		{
			perror("send failed");
			return NULL;
		}
		i++;
		if (i % g_step == 0)
		{
			i = 0;
		}
		usleep(1000000/g_speed);
	}
}

void *RecvThreadProc(void *lp)
{
	int fd = *(int *)lp;
	char recvBuf[MAX_RECV_BUFFER_LEN] = { 0 };
	while (!g_exitFlag)
	{
		int nbytes = read(fd, recvBuf, sizeof(recvBuf));
		printf("recv: %s\n", recvBuf);
		if (nbytes < 0)
		{
			perror("read failed");
			return NULL;
		}
		map<string, string>response = FormStringToMap(recvBuf);

		map<string, string>::iterator iter = response.find("request_id");
		if (iter == response.end())
		{
			printf("response request_id not found\n");
			continue;
		}
		string requestId = iter->second;

		iter = response.find("send_time");
		if (iter == response.end())
		{
			printf("response send_time not found, request_id=%s\n", requestId.c_str());
			continue;
		}
		int64_t sendTime = atoll(iter->second.c_str());

		int64_t costTime = GetTickCount(1) - sendTime;

		printf("request_id: %s costTime: %ld\n",  requestId.c_str(), costTime);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 6)
	{
		printf("invalid parameter\n");
		printf("Usage: ./client [connection] [rangeStart] [rangeEnd] [step] [speed]\n");
		return -1;
	}
	g_exitFlag = false;
	g_reqId = 0;

	srand(time(NULL));   // Initialization, should only be called once.

	int connCnt = atoi(argv[1]);
	g_rangeStart = atoll(argv[2]);
	g_rangeStop = atoll(argv[3]);
	g_step = atoi(argv[4]);
	g_speed = atoi(argv[5]);

	pthread_t *sendThreadId = new pthread_t[connCnt];
	pthread_t *recvThreadId = new pthread_t[connCnt];
	int *fd = new int[connCnt];

	for (int i = 0; i < connCnt; i++)
	{
		fd[i] = ConnectToServer("127.0.0.1", DEFAULT_PORT);
		if (fd[i] < 0)
		{
			printf("connect to server failed, ret=%d\n", fd);
			return -2;
		}

		int ret = pthread_create(&sendThreadId[i], NULL, SendThreadProc, &fd[i]);
		if (ret != 0)
		{
			perror("pthread_create failed");
			return -3;
		}
		ret = pthread_create(&recvThreadId[i], NULL, RecvThreadProc, &fd[i]);
		if (ret != 0)
		{
			perror("pthread_create failed");
			return -4;
		}
	}

	while (!g_exitFlag)
	{
		sleep(10);
	}

	for (int i = 0; i < connCnt; i++)
	{
		int ret = pthread_join(sendThreadId[i], NULL);
		if (ret != 0)
		{
			perror("pthread_join failed");
			return -5;
		}
		ret = pthread_join(recvThreadId[i], NULL);
		if (ret != 0)
		{
			perror("pthread_join failed");
			return -6;
		}
	}
	delete[] sendThreadId;
	delete[] recvThreadId;
	delete[] fd;

	return 0;
}
