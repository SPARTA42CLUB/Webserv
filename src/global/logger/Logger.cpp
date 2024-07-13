#include "Logger.hpp"
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "color.hpp"

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
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
void Logger::logAccept(const int fd, struct sockaddr_in addr) const
{
    std::ostringstream oss;
    oss << "[ Accept Client ]\n"
        << "IP: " << inet_ntoa(addr.sin_addr) << "\n"
        << "Port: " << ntohs(addr.sin_port) << "\n"
        << "Socket: " << fd << "\n";

    log(INFO, oss.str());
}

void Logger::logHttpMessage(const ResponseMessage* res, const RequestMessage* req) const
{
    std::ostringstream oss;
    oss << "[ Request ]\n" << req->toString() << "[ Response ]\n" << res->toString();

    log(INFO, oss.str());
}

void Logger::logHttpMessage(const ResponseMessage* res) const
{
    std::ostringstream oss;
    oss << "[ Response ]\n" << res->toString();

    log(INFO, oss.str());
}

void Logger::logHttpMessage(const RequestMessage* req) const
{
    std::ostringstream oss;
    oss << "[ Request ]\n" << req->toString();

    log(INFO, oss.str());
}

void Logger::logInfo(const std::string& message) const
{
    log(INFO, message);
}

void Logger::log(eLogLevel level, const std::string& message) const
{
    const std::string logDir = (level == INFO) ? ACCESS_LOG_DIR : ERROR_LOG_DIR;
    const std::string logPath = logDir + "/" + getCurrentDate() + ".log";

    // 디렉토리가 존재하지 않으면 생성합니다.
    if (!std::filesystem::exists(logDir))
        std::filesystem::create_directories(logDir);

    std::ofstream logFile(logPath, std::ios::app);

    const std::string logMessage = "[ " + logLevelToString(level) + " ] " + getTimeStamp() + '\n' + message;

    if (level != INFO)
        std::cerr << color::FG_RED << logMessage << color::RESET << std::endl;

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
    time_t now = time(NULL);
    tm* localtm = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtm);
    return std::string(buffer);
}

std::string Logger::getCurrentDate() const
{
    // 현재 날짜와 시간을 가져옵니다.
    std::time_t now = std::time(nullptr);
    std::tm localtm = *std::localtime(&now);

    // 날짜를 문자열로 변환합니다.
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &localtm);
    return std::string(buffer);
}
