#ifndef STATUS_LINE_HPP
#define STATUS_LINE_HPP

#include <string>

class StatusLine
{
private:
    std::string mHTTPVersion;
    std::string mStatusCode; // Response Message를 보낼 때 문자열로 보내기 때문에 string으로 선언
    std::string mReasonPhrase;
    StatusLine(const StatusLine& rhs);
    StatusLine& operator=(const StatusLine& rhs);

public:
    StatusLine();
    ~StatusLine();
    const std::string& getHTTPVersion() const;
    const std::string& getStatusCode() const;
    const std::string& getReasonPhrase() const;
    void setHTTPVersion(const std::string& httpVersion);
    void setStatusCode(const std::string& statusCode);
    void setStatusCode(const int statusCode);
    void setReasonPhrase(const std::string& reasonPhrase);
    const std::string toString() const;
};
#endif
