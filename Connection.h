#pragma once
#include <vector>
#include "Util.h"

class Connection
{
	Connection();
	int read(std::vector<TaskInfo>& tasks);
private:
	char m_recvBuf[MAX_RECV_BUFFER_LEN];
	int m_bufEndPointer;
	int m_socket;
};