#include "HttpException.hpp"
#include <stdio.h>

HttpException::HttpException(const int statusCode)
: mStatusCode(statusCode)
{
}
HttpException::~HttpException() noexcept
{
}
int HttpException::getStatusCode() const
{
    return mStatusCode;
}
