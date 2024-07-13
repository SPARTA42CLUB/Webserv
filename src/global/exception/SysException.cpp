#include "SysException.hpp"

SysException::SysException(enum eReason reason)
: mReason(reason)
{
}
SysException::~SysException() noexcept
{
}
const char* SysException::what() const noexcept
{
    switch (mReason)
    {
        case FAILED_TO_CREATE_EVENT:
            return "Failed to create queue";
        case FAILED_TO_ADD_READ_EVENT:
            return "Failed to add read event";
        case FAILED_TO_ADD_WRITE_EVENT:
            return "Failed to add write event";
        case FAILED_TO_DELETE_READ_EVENT:
            return "Failed to delete read event";
        case FAILED_TO_DELETE_WRITE_EVENT:
            return "Failed to delete write event";
        case FAILED_TO_GET_EVENT:
            return "Failed to get event";
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
        case FAILED_TO_DUP:
            return "Failed to dup2";
        case FAILED_TO_EXEC:
            return "Failed to exec";
        default:
            return "Unknown exception";
    }
}
