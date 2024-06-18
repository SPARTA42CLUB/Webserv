#include "EventManager.hpp"
#include "error.hpp"
#include <unistd.h>

EventManager::EventManager() {

	kq = kqueue();
	if (kq == -1)
		exit_with_error("Failed to create kqueue");
}

EventManager::~EventManager() {
	close(kq);
}

void EventManager::addEvent(int fd, int16_t filter, uint16_t flags) {
	struct kevent evSet;
	EV_SET(&evSet, fd, filter, flags, 0, 0, NULL);

	if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1)
		throw std::runtime_error("Failed to add event to kqueue");
}

std::vector<struct kevent> EventManager::getCurrentEvents() {

	struct timespec timeout;
	timeout.tv_sec = 1;
	timeout.tv_nsec = 0;

	/*
	 * !!수신이 감지된 이벤트를 가져온다!!
	 * events 이벤트 배열에 현재 변경이 감지된 이벤트를 저장한다.
	 * numEvents에는 배열에 담긴 이벤트의 수가 담긴다.
	 */
	struct kevent events[1024];
	int numEvents = kevent(kq, NULL, 0, events, 1024, &timeout);
	if (numEvents == -1)
		throw std::runtime_error("Failed to add event to kqueue");

	return std::vector<struct kevent>(events, events + numEvents);
}

int EventManager::getKqueue() {
	return kq;
}
