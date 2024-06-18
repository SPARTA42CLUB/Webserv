#include "HTTPException.hpp"

HTTPException::HTTPException(const int statusCode)
: mStatusCode(statusCode)
{
}
HTTPException::~HTTPException() _NOEXCEPT
{
}
int HTTPException::getStatusCode() const
{
    return mStatusCode;
}
