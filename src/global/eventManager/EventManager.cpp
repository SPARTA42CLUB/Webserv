#include "EventManager.hpp"
#include <unistd.h>
#include "SysException.hpp"

EventManager& EventManager::getInstance()
{
    static EventManager instance;
    return instance;
}
EventManager::EventManager()
: kq(kqueue())
{
    if (kq == -1)
    {
        throw SysException(FAILED_TO_CREATE_KQUEUE);
    }
}
EventManager::~EventManager()
{
    close(kq);
}
void EventManager::addReadEvent(const int fd)
{
    addEvent(fd, EVFILT_READ, EV_ADD | EV_ENABLE);
}

void EventManager::addWriteEvent(const int fd)
{
    addEvent(fd, EVFILT_WRITE, EV_ADD | EV_ENABLE);
}

void EventManager::deleteReadEvent(const int fd)
{
    addEvent(fd, EVFILT_READ, EV_DELETE);
}

void EventManager::deleteWriteEvent(const int fd)
{
    addEvent(fd, EVFILT_WRITE, EV_DELETE);
}

// throw-safe 함수 (에러 처리 해야 할 수도?)
void EventManager::addEvent(const int fd, const int16_t filter, const uint16_t flags)
{
    struct kevent evSet;
    EV_SET(&evSet, fd, filter, flags, 0, 0, NULL);

    if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1)
    {
        close(fd);
    }
}

std::vector<struct kevent> EventManager::getCurrentEvents()
{
    /*
    kevent는 이벤트가 발생해야 반환되는데,
    timeout을 설정하면 이벤트가 발생하지 않아도
    kevent함수가 반환된다.
    */
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
    // NOTE: cgi_interpreter 경로가 올바르지 않으면 errno(9) bad file descripter 반환됨 왜이럼???????
    // 예상 이유: addEvent할 때 에러가 아예 빼서 그런듯? 뺀 이유는 eventGarbageCollector 때문임!!!!!!!!!!!!!!!!!!!!!
    // 안준성이 만듬 당신이 해
    if (numEvents == -1)
    {
        throw SysException(FAILED_TO_GET_KEVENT);
    }

    return std::vector<struct kevent>(events, events + numEvents);
}
