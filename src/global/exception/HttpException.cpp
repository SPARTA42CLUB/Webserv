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
// WARNING: NOTE: 지워라
const char* HttpException::what() const _NOEXCEPT
{
    char* numberstring = new char[4];
    snprintf(numberstring, 4, "%d", mStatusCode);
    return numberstring;
}
