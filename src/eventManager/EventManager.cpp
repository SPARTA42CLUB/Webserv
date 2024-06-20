#include "EventManager.hpp"
#include <unistd.h>
#include "Exception.hpp"
#include <iostream>

EventManager::EventManager() {

	kq = kqueue();
	if (kq == -1)
    {
        throw Exception(FAILED_TO_CREATE_KQUEUE);
    }
}

EventManager::~EventManager() {
	close(kq);
}

void EventManager::addReadEvent(int socket) {
	addEvent(socket, EVFILT_READ, EV_ADD | EV_ENABLE);
}

void EventManager::addWriteEvent(int socket) {
	addEvent(socket, EVFILT_WRITE, EV_ADD | EV_ENABLE);
}

void EventManager::deleteReadEvent(int socket) {
	addEvent(socket, EVFILT_READ, EV_DELETE);
}

void EventManager::deleteWriteEvent(int socket) {
	addEvent(socket, EVFILT_WRITE, EV_DELETE);
}

void EventManager::addEvent(int socket, int16_t filter, uint16_t flags) {
	struct kevent evSet;
	EV_SET(&evSet, socket, filter, flags, 0, 0, NULL);

	if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1) {
		std::cerr << "kevent failed: " << strerror(errno) << std::endl;
		throw std::runtime_error("Failed to add event to kqueue");
	}
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
		throw std::runtime_error("Failed to get event from kqueue");

	return std::vector<struct kevent>(events, events + numEvents);
}

int EventManager::getKqueue() {
	return kq;
}
