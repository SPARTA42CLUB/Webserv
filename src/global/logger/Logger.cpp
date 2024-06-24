#include "Logger.hpp"
#include <arpa/inet.h>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger()
{
}
Logger::~Logger()
{
}

void Logger::logWarning(const std::string& logMessage)
{
    log(WARNING, logMessage, "error.log");
}

void Logger::logError(const std::string& logMessage)
{
    log(ERROR, logMessage, "error.log");
}

void Logger::logAccept(int socket, struct sockaddr_in addr)
{
	std::ostringstream oss;
	oss << "[ Accept Client ] \n"
			<< "IP: "
			<< inet_ntoa(addr.sin_addr)
			<< "\n"
			<< "Port: "
			<< ntohs(addr.sin_port)
			<< "\n"
            << "Socket: "
            << socket
            << "\n"
			;

	log(INFO, oss.str(), "access.log");
}

void Logger::logHTTPMessage(const ResponseMessage& res, const RequestMessage& req)
{
	std::ostringstream oss;
    oss << "Request:\n"
            << req.toString()
            << "Response:\n"
            << res.toString()
			;

	log(INFO, oss.str(), "access.log");
}

void Logger::log(LogLevel level, const std::string& message, const std::string& filePath) {

	std::ofstream logFile;
    logFile.open(filePath, std::ios::app);
    std::string logMessage = "[ " + logLevelToString(level) + " ] " + getTimeStamp() + "\n" + message + "\n----------------------------------------------------------------------------------------\n";

    if (level == INFO)
        std::cout << logMessage << std::endl;
    else if (level == WARNING || level == ERROR)
        std::cerr << logMessage << std::endl;

    if (logFile.is_open()) {
        logFile << logMessage << std::endl;
        logFile.close();
    }
}

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case INFO: return "INFO";
        case WARNING: return "WARNING";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string Logger::getTimeStamp() {
    std::time_t now = std::time(NULL);
    std::tm* localtm = std::localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtm);
    return std::string(buffer);
}
