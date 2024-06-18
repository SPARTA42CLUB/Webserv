#include "HTTPException.hpp"

HTTPException::HTTPException(int statusCode, const std::string& reasonPhrase)
: mStatusCode(statusCode)
, mReasonPhrase(reasonPhrase)
{
}
HTTPException::~HTTPException() _NOEXCEPT
{
}
const std::string HTTPException::getStatusCode() const
{
    return std::to_string(mStatusCode);
}
const std::string& HTTPException::getReasonPhrase() const
{
    return mReasonPhrase;
}
