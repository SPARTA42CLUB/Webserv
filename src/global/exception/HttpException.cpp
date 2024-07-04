#include "HttpException.hpp"
#include <stdio.h>

HttpException::HttpException(const int statusCode)
: mStatusCode(statusCode)
{
}
HttpException::~HttpException() _NOEXCEPT
{
}
int HttpException::getStatusCode() const
{
    return mStatusCode;
}
