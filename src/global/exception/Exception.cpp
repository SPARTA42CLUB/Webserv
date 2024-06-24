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
        case FAILED_TO_CREATE_KQUEUE:
            return "Failed to create kqueue";
        case KEVENT_ERROR:
            return "Kevent error";
        case FAILED_TO_ADD_KEVENT:
            return "Failed to add kevent";
        case FAILED_TO_GET_KEVENT:
            return "Failed to get kevent";
        case FAILED_TO_SEND:
            return "Failed to send";
        default:
            return "Unknown exception";
    }
}
