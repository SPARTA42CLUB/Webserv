#pragma once

#include <sys/event.h>
#include <vector>

class EventManager {
public:
	EventManager();
	~EventManager();

	void addEvent(int fd, int16_t filter, uint16_t flags);
	std::vector<struct kevent> getCurrentEvents();
	int getKqueue();

private:
	int kq;
};
