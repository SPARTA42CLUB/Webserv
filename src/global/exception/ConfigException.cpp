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
    default:
        return "Unknown exception";
    }
}
