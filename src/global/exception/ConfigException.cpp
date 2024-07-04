#include "ConfigException.hpp"

ConfigException::ConfigException(eConfigException exceptionType)
: mExceptionType(exceptionType)
{
}
ConfigException::~ConfigException() _NOEXCEPT
{
}
const char* ConfigException::what() const _NOEXCEPT
{
    switch (mExceptionType)
    {
    case FAILED_TO_OPEN_CONFIG_FILE:
        return "Failed to open config file";
    case INVALID_CONFIG_FILE:
        return "Invalid config file";
    case INVALID_SERVER_CONFIG:
        return "Invalid server config";
    case INVALID_KEEPALIVE_TIMEOUT:
        return "Invalid keepalive timeout";
    case INVALID_LOCATION_CONFIG:
        return "Invalid location config";
    case EMPTY_SERVER_CONFIG:
        return "Empty Server Config";
    case PORT_NOT_EXIST:
        return "Port not exist in server config";
    case HOST_NOT_EXIST:
        return "Host not exist in server config";
    case ROOT_NOT_EXIST:
        return "Root not exist in location config";
    case DUPLICATE_ROOT_ALIAS:
        return "Duplicate root alias in location config";
    case DUPLICATE_PORT:
        return "Duplicate port in server config";
    default:
        return "Unknown exception in config file";
    }
}
