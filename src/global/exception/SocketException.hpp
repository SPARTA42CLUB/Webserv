#pragma once

#include <exception>

enum eSocketException
{
    FAILED_TO_CREATE_SOCKET,
	FAILED_TO_BIND_SOCKET,
	FAILED_TO_LISTEN_SOCKET,
	FAILED_TO_SET_NON_BLOCKING,
};

class SocketException : public std::exception
{
private:
    enum eSocketException e;

public:
    SocketException(enum eSocketException e);
    virtual ~SocketException() _NOEXCEPT;
    const char* what() const _NOEXCEPT;
};
