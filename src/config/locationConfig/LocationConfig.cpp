#include "LocationConfig.hpp"
#include <sstream>
#include "ConfigException.hpp"
#include "parse.hpp"

const size_t LocationConfig::implementMethodsSize = 4;
const std::string LocationConfig::implementMethods[implementMethodsSize] = {"GET", "HEAD", "POST", "DELETE"};

LocationConfig::LocationConfig()
: root()
, index()
, allow_methods()
, directory_listing(false)
, redirect()
, cgi_interpreter()
{
}
LocationConfig::~LocationConfig()
{
}
LocationConfig::LocationConfig(const LocationConfig& rhs)
: root(rhs.root)
, index(rhs.index)
, allow_methods(rhs.allow_methods)
, directory_listing(rhs.directory_listing)
, redirect(rhs.redirect)
, cgi_interpreter(rhs.cgi_interpreter)
{
}
LocationConfig& LocationConfig::operator=(const LocationConfig& rhs)
{
    if (this == &rhs)
        return *this;
    root = rhs.root;
    index = rhs.index;
    allow_methods = rhs.allow_methods;
    directory_listing = rhs.directory_listing;
    redirect = rhs.redirect;
    cgi_interpreter = rhs.cgi_interpreter;
    return *this;
}
void LocationConfig::parseRoot(std::string& value)
{
    if (!isValidValue(value))
        throw ConfigException(INVALID_LOCATION_CONFIG);
    root = value;
}
void LocationConfig::parseIndex(std::string& value)
{
    if (!isValidValue(value))
        throw ConfigException(INVALID_LOCATION_CONFIG);
    index = value;
}
void LocationConfig::parseAllowMethods(std::string& value)
{
    if (value.back() != ';')
        throw ConfigException(INVALID_LOCATION_CONFIG);
    value.pop_back();
    std::istringstream iss(value);
    std::string method;
    while (iss >> method)
    {
        if (allow_methods[method] == true)
            throw ConfigException(INVALID_LOCATION_CONFIG);
        if (std::find(implementMethods, implementMethods + implementMethodsSize, method) == implementMethods + implementMethodsSize)
            throw ConfigException(INVALID_LOCATION_CONFIG);
        allow_methods[method] = true;
    }
    if (allow_methods.empty())
        throw ConfigException(INVALID_LOCATION_CONFIG);
}
void LocationConfig::parseDirectoryListing(std::string& value)
{
    if (!isValidValue(value))
        throw ConfigException(INVALID_LOCATION_CONFIG);
    if (value == "on")
        directory_listing = true;
    else if (value == "off")
        directory_listing = false;
    else
        throw ConfigException(INVALID_LOCATION_CONFIG);
}
void LocationConfig::parseRedirect(std::string& value)
{
    if (!isValidValue(value))
        throw ConfigException(INVALID_LOCATION_CONFIG);
    redirect = value;
}
void LocationConfig::parseCGI(std::string& value)
{
    if (!isValidValue(value))
        throw ConfigException(INVALID_LOCATION_CONFIG);
    cgi_interpreter = value;
}
