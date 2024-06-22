#include "Connection.hpp"

#include <unistd.h>

Connection::Connection(int serverSocket) : last_activity(time(NULL)), isChunked(false)
{

    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    int connection = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
    if (connection == -1)
    {
        std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
        return;
    }

    setNonBlocking(connection);
}
Connection::~Connection()
{
    close(fd);

	for (size_t i = 0; i < responses.size(); ++i) {
		delete responses[i]; // 메모리 해제
	}
}

int Connection::getFd() {
	return fd;
}

void Connection::update_last_activity()
{
    last_activity = time(NULL);
}
