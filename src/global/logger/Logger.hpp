#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <netinet/in.h>

#include "ResponseMessage.hpp"
#include "RequestMessage.hpp"

class Logger {
public:
    enum LogLevel {
        INFO,
        WARNING,
        ERROR
    };

	// Singleton
    static Logger& getInstance();

    void logWarning(const std::string& logMessage);
    void logError(const std::string& logMessage);
	void logAccept(int socket, struct sockaddr_in addr);
	void logHTTPMessage(const ResponseMessage& res, const RequestMessage& req);


private:
    // Singleton
    Logger();
    ~Logger();
    Logger(const Logger&);
    Logger& operator=(const Logger&);

    void log(LogLevel level, const std::string& message, const std::string& filePath);

    std::string logLevelToString(LogLevel level);
    std::string getTimeStamp();
};
