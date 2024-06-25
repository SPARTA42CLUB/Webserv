#ifndef START_LINE_HPP
#define START_LINE_HPP

#include <string>



class StartLine
{
private:
    std::string mElements[3];
    enum eStartLine
    {
        HTTP_VERSION = 0,
        STATUS_CODE = 1,
        REASON_PHRASE = 2,
        METHOD = 1,
        REQUEST_TARGET = 2
    };
    StartLine(const StartLine& rhs);
    StartLine& operator=(const StartLine& rhs);

public:
    StartLine();
    ~StartLine();
    const std::string& getHTTPVersion() const;
    const std::string& getStatusCode() const;
    const std::string& getReasonPhrase() const;
    const std::string& getMethod() const;
    const std::string& getRequestTarget() const;
    void setHTTPVersion(const std::string& httpVersion);
    void setStatusCode(const std::string& statusCode);
    void setStatusCode(const int statusCode);
    void setReasonPhrase(const std::string& reasonPhrase);
    void parseRequestLine(const std::string& requestLine);
    const std::string toString() const;
};
#endif
