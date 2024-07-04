#ifndef SYSTEM_CALL_EXCEPTION_HPP
#define SYSTEM_CALL_EXCEPTION_HPP

#include <exception>

enum eReason
{
    FAILED_TO_CREATE_KQUEUE,
    FAILED_TO_ADD_READ_KEVENT,
    FAILED_TO_ADD_WRITE_KEVENT,
    FAILED_TO_DELETE_READ_KEVENT,
    FAILED_TO_DELETE_WRITE_KEVENT,
    FAILED_TO_GET_KEVENT,
    KEVENT_ERROR,
    FAILED_TO_SEND,
    FAILED_TO_CREATE_SOCKET,
	FAILED_TO_BIND_SOCKET,
	FAILED_TO_LISTEN_SOCKET,
	FAILED_TO_SET_NON_BLOCKING,
    FAILED_TO_CREATE_PIPE,
    FAILED_TO_FORK,
    FAILED_TO_DUP,
    FAILED_TO_EXEC,
};

class SysException : public std::exception
{
private:
    enum eReason mReason;

public:
    SysException(enum eReason reason);
    virtual ~SysException() _NOEXCEPT;
    virtual const char* what() const _NOEXCEPT;
};

#endif
