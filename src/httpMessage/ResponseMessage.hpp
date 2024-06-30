#ifndef RESPONSE_MESSAGE_HPP
#define RESPONSE_MESSAGE_HPP

#include <sstream>
#include "HeaderFields.hpp"
#include "MessageBody.hpp"
#include "StartLine.hpp"
#include "ServerConfig.hpp"
#include "RequestMessage.hpp"

class ResponseMessage
{
private:
    StartLine mStatusLine;
    HeaderFields mResponseHeaderFields;
    MessageBody mMessageBody;

    void parseStatusLine(std::istringstream& resStream);
    void parseResponseHeaderFields(std::istringstream& resStream);
    void parseMessageBody(std::istringstream& resStream);
    std::string getErrorMsgBody(const int statusCode, const ServerConfig& serverConfig) const;

    ResponseMessage(const ResponseMessage& rhs);
    ResponseMessage& operator=(const ResponseMessage& rhs);

public:
    ResponseMessage();
    ~ResponseMessage();
    void parseResponseMessage(const std::string& response);
    void setStatusLine(const std::string& httpVersion, const std::string& statusCode, const std::string& reasonPhrase);
    void setStatusLine(const std::string& httpVersion, const int statusCode, const std::string& reasonPhrase);
    void addResponseHeaderField(const std::string& key, const std::string& value);
    void addResponseHeaderField(const std::string& key, const int value);
    void addSemanticHeaderFields(void);
    void addMessageBody(const std::string& body);

    void setByStatusCode(const int statusCode, const ServerConfig& serverConfig);
    const HeaderFields& getResponseHeaderFields() const;
    size_t getMessageBodySize() const;
    void clearMessageBody();
    std::string toString(void) const;

    // 4xx
    void badRequest(const std::string& body = "");
    void forbidden(const std::string& body = "");
    void notFound(const std::string& body = "");
    void methodNotAllowed(const std::string& body = "");
    void uriTooLong(const std::string& body = "");
    void contentTooLarge(const std::string& body = "");
    // 5xx
    void badGateway(const std::string& body = "");
    void serviceUnavailable(const std::string& body = "");
    void httpVersionNotSupported(const std::string& body = "");
};

#endif
