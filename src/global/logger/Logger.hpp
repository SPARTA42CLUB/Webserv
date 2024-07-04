#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"

class Logger
{
public:
    enum eLogLevel
    {
        INFO,
        WARNING,
        ERROR
    };

    // Singleton
    static Logger& getInstance();

    void logInfo(const std::string& message) const;
    void logWarning(const std::string& logMessage) const;
    void logError(const std::string& logMessage) const;
    void logAccept(const int fd, struct sockaddr_in addr) const;
    void logHttpMessage(const ResponseMessage* res, const RequestMessage* req) const;
    void logHttpMessage(const ResponseMessage* res) const;
    void logHttpMessage(const RequestMessage* req) const;

private:
    const std::string accessLogPath;
    const std::string errorLogPath;

    // Singleton
    Logger();
    ~Logger();
    Logger(const Logger&);
    Logger& operator=(const Logger&);

    void log(eLogLevel level, const std::string& message) const;

    std::string logLevelToString(const eLogLevel level) const;
    std::string getTimeStamp() const;
};

#endif
