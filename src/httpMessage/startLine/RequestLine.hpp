#ifndef REQUEST_LINE_HPP
#define REQUEST_LINE_HPP

#include <string>

class RequestLine
{
private:
    std::string mMethod;
    std::string mRequestTarget;
    std::string mHTTPVersion;
    RequestLine(const RequestLine& rhs);
    RequestLine& operator=(const RequestLine& rhs);

public:
    RequestLine();
    ~RequestLine();
    void parseRequestLine(const std::string& requestLine);
    std::string getMethod(void) const;
    std::string getRequestTarget(void) const;
    std::string getHTTPVersion(void) const;
};

#endif
