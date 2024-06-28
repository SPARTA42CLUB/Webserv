#ifndef RESPONSE_MESSAGE_HPP
#define RESPONSE_MESSAGE_HPP

#include <sstream>
#include "HeaderFields.hpp"
#include "MessageBody.hpp"
#include "StartLine.hpp"

class ResponseMessage
{
private:
    StartLine mStatusLine;
    HeaderFields mResponseHeaderFields;
    MessageBody mMessageBody;
    ResponseMessage(const ResponseMessage& rhs);
    ResponseMessage& operator=(const ResponseMessage& rhs);

    void addSemanticHeaderFields();

public:
    ResponseMessage();
    ~ResponseMessage();
    void setStatusLine(const std::string& httpVersion, const std::string& statusCode, const std::string& reasonPhrase);
    void setStatusLine(const std::string& httpVersion, const int statusCode, const std::string& reasonPhrase);
    void addResponseHeaderField(const std::string& key, const std::string& value);
    void addResponseHeaderField(const std::string& key, const int value);
    void addMessageBody(const std::string& body);
    std::string toString(void) const;
    size_t getMessageBodySize() const;
    void clearMessageBody();


    void setByStatusCode(const int statusCode);

    // 4xx
    void badRequest(void);
    void forbidden(void);
    void notFound(void);
    void methodNotAllowed(void);
    void uriTooLong(void);
    void payloadTooLarge();
    // 5xx
    void httpVersionNotSupported(void);
};

#endif
