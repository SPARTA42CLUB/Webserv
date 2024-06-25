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
#include <iostream>
#include <cstdio>
const char* HTTPException::what() const _NOEXCEPT
{
    char* str = new char[20];
    sprintf(str, "%d", mStatusCode);
    return str;
}
