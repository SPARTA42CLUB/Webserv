#ifndef REQUEST_MESSAGE_HPP
#define REQUEST_MESSAGE_HPP

#include <sstream>
#include "MessageBody.hpp"
#include "HeaderFields.hpp"
#include "RequestLine.hpp"

class RequestMessage
{
private:
    RequestLine mRequestLine;
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
};

#endif
