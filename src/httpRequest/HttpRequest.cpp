#include "HttpRequest.hpp"

HttpRequest::HttpRequest(const std::string& requestData)
{
	(void)requestData;
}
std::string HttpRequest::getPath() const
{
	return "/";
}
std::string HttpRequest::getMethod() const
{
	return "GET";
}
