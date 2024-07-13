#include "EventManager.hpp"
#include <unistd.h>
#include "SysException.hpp"

EventManager& EventManager::getInstance()
{
    static EventManager instance;
    return instance;
}
EventManager::EventManager()
: EVENT_FD(EVENT_CREATE())
{
    if (EVENT_FD == -1)
    {
        throw SysException(FAILED_TO_CREATE_EVENT);
    }
}
EventManager::~EventManager()
{
    close(EVENT_FD);
}
void EventManager::addReadEvent(const int fd)
{
    addEvent(fd, READ_EVENT, ADD_EVENT);
}

void EventManager::addWriteEvent(const int fd)
{
    addEvent(fd, WRITE_EVENT, ADD_EVENT);
}

void EventManager::modReadEvent(const int fd)
{
    addEvent(fd, READ_EVENT, MOD_EVENT);
}

void EventManager::modWriteEvent(const int fd)
{
    addEvent(fd, WRITE_EVENT, MOD_EVENT);
}

void EventManager::deleteReadEvent(const int fd)
{
    addEvent(fd, READ_EVENT, DEL_EVENT);
}

void EventManager::deleteWriteEvent(const int fd)
{
    addEvent(fd, WRITE_EVENT, DEL_EVENT);
}

void EventManager::addEvent(const int fd, const int16_t filter, const uint16_t flags)
{
#ifdef __linux__
    struct epoll_event ev;
    ev.events = filter;
    ev.data.fd = fd;
    if (epoll_ctl(EVENT_FD, flags, fd, &ev) == -1)
    {
        close(fd);
    }
#else
    struct EVENT_TYPE evSet;
    EV_SET(&evSet, fd, filter, flags, 0, 0, NULL);

    if (kevent(EVENT_FD, &evSet, 1, NULL, 0, NULL) == -1)
    {
        close(fd);
    }
#endif
}

std::vector<struct EVENT_TYPE> EventManager::getCurrentEvents()
{
    struct EVENT_TYPE events[1024];
#ifdef __linux__
    int timeout = 1000; // 1000ms timeout
    int numEvents = epoll_wait(EVENT_FD, events, 1024, timeout);
#else
    struct timespec timeout;
    timeout.tv_sec = 1;
    timeout.tv_nsec = 0;

    int numEvents = kevent(EVENT_FD, NULL, 0, events, 1024, &timeout);
#endif

    if (numEvents == -1)
    {
        throw SysException(FAILED_TO_GET_EVENT);
    }

    return std::vector<struct EVENT_TYPE>(events, events + numEvents);
}
