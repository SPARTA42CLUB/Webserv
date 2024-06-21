#ifndef REQUEST_MESSAGE_HPP
#define REQUEST_MESSAGE_HPP

#include <sstream>
#include "HeaderFields.hpp"
#include "MessageBody.hpp"
#include "StartLine.hpp"

class RequestMessage
{
private:
    StartLine mRequestLine;
    HeaderFields mRequestHeaderFields;
    MessageBody mMessageBody;
    void parseRequestMessage(const std::string& request);
    void parseRequestLine(std::istringstream& reqStream);
    void parseRequestHeaderFields(std::istringstream& reqStream);
    void parseMessageBody(std::istringstream& reqStream);
    RequestMessage(const RequestMessage& rhs);
    RequestMessage& operator=(const RequestMessage& rhs);

public:
    RequestMessage();
    RequestMessage(const std::string& request);
    ~RequestMessage();
    const StartLine& getRequestLine() const;
    const HeaderFields& getRequestHeaderFields() const;
    const MessageBody& getMessageBody() const;
};

#endif
