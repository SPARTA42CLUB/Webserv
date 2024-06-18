#ifndef RESPONSE_MESSAGE_HPP
#define RESPONSE_MESSAGE_HPP

#include <sstream>
#include "MessageBody.hpp"
#include "HeaderFields.hpp"
#include "StatusLine.hpp"

class ResponseMessage
{
private:
    StatusLine mStatusLine;
    HeaderFields mResponseHeaderFields;
    MessageBody mMessageBody;
    ResponseMessage(const ResponseMessage& rhs);
    ResponseMessage& operator=(const ResponseMessage& rhs);

public:
    ResponseMessage();
    ~ResponseMessage();
    void setStatusLine(const std::string& httpVersion, const std::string& statusCode, const std::string& reasonPhrase);
    void setStatusLine(const std::string& httpVersion, const int statusCode, const std::string& reasonPhrase);
    void addResponseHeaderField(const std::string& key, const std::string& value);
    void addResponseHeaderField(const std::string& key, int value);
    void addMessageBody(const std::string& body);
    std::string toString(void) const;
    size_t getMessageBodySize() const;
    void clearMessageBody();
};

#endif
