#pragma once

#ifdef __linux__
#include <sys/epoll.h>
#define EVENT_TYPE epoll_event
#define EVENT_CREATE() epoll_create1(0)
#define READ_EVENT EPOLLIN
#define WRITE_EVENT EPOLLOUT
#define ADD_EVENT EPOLL_CTL_ADD
#define DEL_EVENT EPOLL_CTL_DEL
#else
#include <sys/event.h>
#define EVENT_TYPE kevent
#define EVENT_CREATE() kqueue()
#define EVENT_FD kq
#define READ_EVENT EVFILT_READ
#define WRITE_EVENT EVFILT_WRITE
#define ADD_EVENT EV_ADD | EV_ENABLE
#define DEL_EVENT EV_DELETE
#endif

inline bool eventError(const struct EVENT_TYPE& ev) {
#ifdef __linux__
    return ev.events & EPOLLERR;
#else
    return ev.flags & EV_ERROR;
#endif
}

inline bool eventFilter(const struct EVENT_TYPE& ev, int filter) {
#ifdef __linux__
    return ev.events & filter;
#else
    return ev.filter == filter;
#endif
}

inline int eventIdent(const struct EVENT_TYPE& ev) {
#ifdef __linux__
    return ev.data.fd;
#else
    return ev.ident;
#endif
}

inline bool eventEOF(const struct EVENT_TYPE& ev) {
#ifdef __linux__
    return ev.events & EPOLLHUP;
#else
    return ev.flags & EV_EOF;
#endif
}

#include <vector>

const int READ_END = 0;
const int WRITE_END = 1;

class EventManager
{
private:
    const int EVENT_FD;

    // Singleton
    EventManager();
    ~EventManager();
    EventManager(const EventManager&);
    EventManager& operator=(const EventManager&);

    void addEvent(const int fd, const int16_t filter, const uint16_t flags);

public:
    // Singleton
    static EventManager& getInstance();

    std::vector<struct EVENT_TYPE> getCurrentEvents();

    void addReadEvent(const int fd);
    void addWriteEvent(const int fd);
    void deleteReadEvent(const int fd);
    void deleteWriteEvent(const int fd);
};
