#pragma once

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>

class HttpResponse {
public:
    HttpResponse();

    void setStatusCode(int code, const std::string& message);

    void setHeader(const std::string& field, const std::string& value);

    void setBody(const std::string& bodyContent);

    std::string toString() const;

private:
    int statusCode;
    std::string statusMessage;
    std::map<std::string, std::string> headers;
    std::string body;
};

#endif
