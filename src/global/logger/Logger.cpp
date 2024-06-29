#include "Logger.hpp"
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include "color.hpp"

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
: accessLogPath("log/access.log")
, errorLogPath("log/error.log")
{
}
Logger::~Logger()
{
}
void Logger::logWarning(const std::string& logMessage) const
{
    log(WARNING, logMessage);
}
void Logger::logError(const std::string& logMessage) const
{
    log(ERROR, logMessage);
}
void Logger::logAccept(int socket, struct sockaddr_in addr) const
{
    std::ostringstream oss;
    oss << "[ Accept Client ]\n"
        << "IP: " << inet_ntoa(addr.sin_addr) << "\n"
        << "Port: " << ntohs(addr.sin_port) << "\n"
        << "Socket: " << socket << "\n";

    log(INFO, oss.str());
}

void Logger::logHttpMessage(const ResponseMessage& res, const RequestMessage& req) const
{
    std::ostringstream oss;
    oss << "[ Request ]\n" << req.toString() << "[ Response ]\n" << res.toString();

    log(INFO, oss.str());
}

void Logger::logHttpMessage(const ResponseMessage& res) const
{
    std::ostringstream oss;
    oss << "[ Response ]\n" << res.toString();

    log(INFO, oss.str());
}

void Logger::logHttpMessage(const RequestMessage& req) const
{
    std::ostringstream oss;
    oss << "[ Request ]\n" << req.toString();

    log(INFO, oss.str());
}

void Logger::logInfo(const std::string& message) const
{
    log(INFO, message);
}

void Logger::log(eLogLevel level, const std::string& message) const
{
    const std::string logPath = (level == INFO) ? accessLogPath : errorLogPath;
    std::ofstream logFile(logPath, std::ios::app);

    const std::string logMessage = "[ " + logLevelToString(level) + " ] " + getTimeStamp() + '\n' + message;

    // if (level == INFO)
    //     std::cout << logMessage << std::endl;
    // else
    //     std::cerr << color::FG_RED << logMessage << color::RESET << std::endl;

    if (logFile.is_open())
    {
        logFile << logMessage << + "\n-----------------------------------\n" << std::endl;
        logFile.close();
    }
}

std::string Logger::logLevelToString(const eLogLevel level) const
{
    switch (level)
    {
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

std::string Logger::getTimeStamp() const
{
    std::time_t now = std::time(NULL);
    std::tm* localtm = std::localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtm);
    return std::string(buffer);
}
