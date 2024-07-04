#pragma once

#include <sys/event.h>
#include <vector>

const int READ_END = 0;
const int WRITE_END = 1;

class EventManager
{
private:
    const int kq;

    // Singleton
    EventManager();
    ~EventManager();
    EventManager(const EventManager&);
    EventManager& operator=(const EventManager&);

    void addEvent(const int fd, const int16_t filter, const uint16_t flags);

public:
    // Singleton
    static EventManager& getInstance();

    std::vector<struct kevent> getCurrentEvents();

    void addReadEvent(const int fd);
    void addWriteEvent(const int fd);
    void deleteReadEvent(const int fd);
    void deleteWriteEvent(const int fd);
};
