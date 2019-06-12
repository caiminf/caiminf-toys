#include "Connection.h"
#include <unistd.h>

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


}