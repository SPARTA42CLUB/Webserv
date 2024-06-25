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
	const int kq;

	void addEvent(const int socket, const int16_t filter, const uint16_t flags);
};
