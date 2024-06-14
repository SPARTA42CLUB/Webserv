#pragma once

#include <string>

class HttpRequest {
public:
	HttpRequest(const std::string& requestData);
	std::string getPath() const;
	std::string getMethod() const;
};
