#include "SocketException.hpp"

SocketException::SocketException(enum eSocketException e)
: e(e)
{
}
SocketException::~SocketException() _NOEXCEPT
{
}
const char* SocketException::what() const _NOEXCEPT
{
    switch (e)
    {
        case FAILED_TO_CREATE_SOCKET:
            return "Failed to create socket";
		case FAILED_TO_BIND_SOCKET:
			return "Failed to bind socket";
		case FAILED_TO_LISTEN_SOCKET:
			return "Failed to listen socket";
		case FAILED_TO_SET_NON_BLOCKING:
			return "Failed to set non-blocking";
        default:
            return "Unknown exception";
    }
}
