#pragma once

#include <sys/event.h>
#include <vector>

class EventManager {
public:
	EventManager();
	~EventManager();

	std::vector<struct kevent> getCurrentEvents();

	void addReadEvent(int socket);
	void addWriteEvent(int socket);
	void deleteReadEvent(int socket);
	void deleteWriteEvent(int socket);

private:
	int kq;

	void addEvent(int socket, int16_t filter, uint16_t flags);
};
