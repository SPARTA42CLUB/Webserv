#include "HttpResponse.hpp"
#include <sstream>

HttpResponse::HttpResponse() : statusCode(200), statusMessage("OK") {}

void HttpResponse::setStatusCode(int code, const std::string& message) {
    statusCode = code;
    statusMessage = message;
}

void HttpResponse::setHeader(const std::string& field, const std::string& value) {
    headers[field] = value;
}

void HttpResponse::setBody(const std::string& bodyContent) {
    body = bodyContent;
    setHeader("Content-Length", std::to_string(body.size()));
}

std::string HttpResponse::toString() const {
    std::stringstream responseStream;
    responseStream << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        responseStream << it->first << ": " << it->second << "\r\n";
    }

    responseStream << "\r\n" << body;

    return responseStream.str();
}
