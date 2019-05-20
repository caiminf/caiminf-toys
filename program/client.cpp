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

void *ClientThreadProc(void *lp)
{
	int64_t calNum = *(int64_t*)lp;
	int sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockFd < 0)
	{
		perror("create socket error");
		return NULL;
	}

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(DEFAULT_PORT);
	int ret = inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
	if (ret <= 0)
	{
		perror("inet_pton failed");
		return NULL;
	}

	ret = connect(sockFd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	if (ret < 0)
	{
		perror("connect failed");
		return NULL;
	}
	

	char recvBuf[MAX_RECV_BUFFER_LEN] = { 0 };
	char sendBuf[MAX_SEND_BUFFER_LEN] = { 0 };

	map<string, string> reqMap = {
		{ "request_id", "00001" },
		{ "iterate_times", std::to_string(calNum) }
	};
	string reqStr = FormRequestString(reqMap);
	strncpy(sendBuf, reqStr.c_str(), MAX_SEND_BUFFER_LEN);

	printf("send msg to server: %s\n", sendBuf);
	ret = send(sockFd, sendBuf, strnlen(sendBuf, MAX_SEND_BUFFER_LEN), 0);
	if (ret < 0)
	{
		perror("send failed");
		return NULL;
	}
	int nbytes = read(sockFd, recvBuf, sizeof(recvBuf));
	if (nbytes < 0)
	{
		perror("read failed");
		return NULL;
	}
	printf("recv: %s\n", recvBuf);
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

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		printf("invalid parameter\n");
		printf("Usage: ./client [connection] [rangeStart] [rangeEnd]\n");
		return -1;
	}

	srand(time(NULL));   // Initialization, should only be called once.

	int connCnt = atoi(argv[1]);
	int64_t rangeStart = atoll(argv[2]);
	int64_t rangeStop = atoll(argv[3]);

	vector<int64_t> calNum;
	RandomVectorGen(rangeStart, rangeStop, connCnt, calNum);

	pthread_t *tid = new pthread_t[connCnt];
	for (int i = 0; i < connCnt; i++)
	{
		int ret = pthread_create(&tid[i], NULL, ClientThreadProc, &calNum[i]);
		if (ret != 0)
		{
			perror("pthread_create failed");
			return -2;
		}
	}
	for (int i = 0; i < connCnt; i++)
	{
		int ret = pthread_join(tid[i], NULL);
		if (ret != 0)
		{
			perror("pthread_join failed");
			return -3;
		}
	}
	delete[] tid;
	return 0;
}
