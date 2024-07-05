#ifndef CONFIG_EXCEPTION_HPP
#define CONFIG_EXCEPTION_HPP

#include <exception>

enum eConfigException
{
    FAILED_TO_OPEN_CONFIG_FILE,
    INVALID_CONFIG_FILE,
    INVALID_SERVER_CONFIG,
    INVALID_KEEPALIVE_TIMEOUT,
    INVALID_LOCATION_CONFIG,
    EMPTY_SERVER_CONFIG,
    PORT_NOT_EXIST,
    HOST_NOT_EXIST,
    ROOT_NOT_EXIST,
    LOCATION_NOT_EXIST,
    DUPLICATE_ROOT_ALIAS,
    DUPLICATE_PORT
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
