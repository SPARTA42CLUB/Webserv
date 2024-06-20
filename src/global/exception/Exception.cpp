#include "Exception.hpp"

Exception::Exception(enum eReason reason)
: mReason(reason)
{
}
Exception::~Exception() _NOEXCEPT
{
}
const char* Exception::what() const _NOEXCEPT
{
    switch (mReason)
    {
        case FAILED_TO_OPEN_CONFIG_FILE:
            return "Failed to open config file";
        case FAILED_TO_CREATE_KQUEUE:
            return "Failed to create kqueue";
        case FAILED_TO_BIND_SOCKET:
            return "Failed to bind socket";
        case FAILED_TO_LISTEN_SOCKET:
            return "Failed to listen socket";
        default:
            return "Unknown exception";
    }
}
