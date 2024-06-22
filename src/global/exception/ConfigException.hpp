#ifndef CONFIG_EXCEPTION_HPP
#define CONFIG_EXCEPTION_HPP

#include <exception>

enum eConfigException
{
    FAILED_TO_OPEN_CONFIG_FILE,
    INVALID_CONFIG_FILE,
    EMPTY_SERVER_CONFIG
};

class ConfigException : public std::exception
{
private:
    eConfigException mExceptionType;

public:
    ConfigException(eConfigException exceptionType);
    virtual ~ConfigException() _NOEXCEPT;
    virtual const char* what() const _NOEXCEPT;
};

#endif
