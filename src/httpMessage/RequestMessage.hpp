#ifndef REQUEST_MESSAGE_HPP
#define REQUEST_MESSAGE_HPP

#include <sstream>
#include "HeaderFields.hpp"
#include "MessageBody.hpp"
#include "StartLine.hpp"

// nginx max uri length
const int MAX_URI_LENGTH = 8200;

class RequestMessage
{
private:
    StartLine mRequestLine;
    HeaderFields mRequestHeaderFields;
    MessageBody mMessageBody;
    void parseRequestLine(std::istringstream& reqStream);
    void parseRequestHeaderFields(std::istringstream& reqStream);
    void verifyRequestLine(void) const;
    void verifyRequestHeaderFields(void) const;

    RequestMessage(const RequestMessage& rhs);
    RequestMessage& operator=(const RequestMessage& rhs);

public:
    RequestMessage();
    ~RequestMessage();
    const StartLine& getRequestLine() const;
    const HeaderFields& getRequestHeaderFields() const;
    const MessageBody& getMessageBody() const;
    void parseRequestHeader(const std::string& request);
    void addMessageBody(const std::string& body);
    std::string toString(void) const;
    size_t getContentLength(void) const;
};

#endif
