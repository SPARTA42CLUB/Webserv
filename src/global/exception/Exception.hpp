#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <exception>

enum eReason
{
    FAILED_TO_CREATE_KQUEUE,
    FAILED_TO_BIND_SOCKET,
    FAILED_TO_LISTEN_SOCKET,
};

class Exception : public std::exception
{
private:
    enum eReason mReason;

public:
    Exception(enum eReason reason);
    virtual ~Exception() _NOEXCEPT;
    const char* what() const _NOEXCEPT;
};

#endif
