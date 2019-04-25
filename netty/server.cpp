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

#include "Util.h"

using std::map;
using std::string;

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

int ProcessRecvBuf(char *inputBuf, int inputBufLen, char *outputBuf, 
					int outputBufMaxLen, int& outputBufLen)
{
	int64_t tcStart = GetTickCount(1);
	map<string, string> reqMap = FormStringToMap(string(inputBuf));
	int64_t num = atoll(reqMap["num"].c_str());
	const int pace = 100000000;
	int tmpPace = pace;
	for (int64_t i = 0; i < num; i++)
	{
		if (--tmpPace == 0)
		{
			int64_t tc = GetTickCount(1);
			printf("num_remain = %ld, already cost %ldms\n", num - i, tc - tcStart);
			tmpPace = pace;
		}
	}
	int64_t tcEnd = GetTickCount(1);
	int64_t cost = tcEnd - tcStart;
	outputBufLen = snprintf(outputBuf, outputBufMaxLen, "request_id=%s&costTime=%ld\n", reqMap["request_id"].c_str(), cost);
	return 0;
}

void *ConnectThreadProc(void *lp)
{
	int *p = (int *)lp;
	int clientFd = *p;
	delete p;

	char recvBuf[MAX_RECV_BUFFER_LEN] = { 0 };
	int nbytes = read(clientFd, recvBuf, sizeof(recvBuf));

	char sendBuf[MAX_SEND_BUFFER_LEN] = { 0 };
	int resultLen = 0;
	int ret = ProcessRecvBuf(recvBuf, nbytes, sendBuf, sizeof(sendBuf), resultLen);
	if (ret != 0)
	{
		printf("process recv buf failed ret=%d\n", ret);
		close(clientFd);
		return NULL;
	}
	nbytes = write(clientFd, sendBuf, resultLen);
	close(clientFd);
	return NULL;
}

int main()
{
	int listenFd = CreateTcpSocket(DEFAULT_PORT);
	if (listenFd < 0)
	{
		printf("create socket failed ret=%d\n", listenFd);
		return -1;
	}

	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);

	while (true)
	{
		int *clientFdPointer = new int;
		*clientFdPointer = accept(listenFd, (struct sockaddr *)&clientAddr, &addrLen);
		printf("accept client, ip: %s, port: %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		pthread_t tid;
		int ret = pthread_create(&tid, NULL, ConnectThreadProc, clientFdPointer);
		if (0 != ret)
		{
			perror("create connect thread fail");
			close(*clientFdPointer);
			delete clientFdPointer;
		}
	}
	return 0;
}
