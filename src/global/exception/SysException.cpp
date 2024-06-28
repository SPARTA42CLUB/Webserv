#include "SysException.hpp"

SysException::SysException(enum eReason reason)
: mReason(reason)
{
}
SysException::~SysException() _NOEXCEPT
{
}
const char* SysException::what() const _NOEXCEPT
{
    switch (mReason)
    {
        case FAILED_TO_CREATE_KQUEUE:
            return "Failed to create kqueue";
        case FAILED_TO_ADD_KEVENT:
            return "Failed to add kevent";
        case FAILED_TO_GET_KEVENT:
            return "Failed to get kevent";
        case KEVENT_ERROR:
            return "Kevent error";
        case FAILED_TO_SEND:
            return "Failed to send";
        case FAILED_TO_CREATE_SOCKET:
            return "Failed to create socket";
		case FAILED_TO_BIND_SOCKET:
			return "Failed to bind socket";
		case FAILED_TO_LISTEN_SOCKET:
			return "Failed to listen socket";
		case FAILED_TO_SET_NON_BLOCKING:
			return "Failed to set non-blocking";
        case FAILED_TO_CREATE_PIPE:
            return "Failed to create pipe";
        case FAILED_TO_FORK:
            return "Failed to fork";
        case FAILED_TO_EXEC:
            return "Failed to exec";
        default:
            return "Unknown exception";
    }
}
