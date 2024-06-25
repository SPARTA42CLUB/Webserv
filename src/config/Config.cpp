#include "Config.hpp"
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "ConfigException.hpp"
#include "color.hpp"

static bool isWhitespace(char c);
static void trim(std::string &str);
static bool isDigitStr(const std::string &str);

const size_t Config::implementMethodsSize = 4;
const std::string Config::implementMethods[implementMethodsSize] = {"GET", "HEAD", "POST", "DELETE"};

Config::Config(const std::string& configFilePath)
: keepalive_timeout(DEFAULT_TIMEOUT)
, serverConfigs()
{
    try
    {
        parse(configFilePath);
        verifyConfig();
    }
    catch(const ConfigException& e)
    {
        throw e;
    }

}

// host 값으로 ServerConfig 찾아서 반환.
const ServerConfig& Config::getServerConfigByHost(std::string host) const
{
    for (std::vector<ServerConfig>::const_iterator it = serverConfigs.begin(); it != serverConfigs.end(); ++it)
    {
        if (it->host == host)
        {
            return *it;
        }
    }

    // 일치하는 host 없으면 기본 serverConfig 반환
    return getDefaultServerConfig();
}

// serverConfigs의 첫 번째 애를 기본 서버로 반환
const ServerConfig& Config::getDefaultServerConfig() const
{
    return serverConfigs.front();
}
// Config 파일 파싱
void Config::parse(const std::string& configFilePath)
{
    std::ifstream file(configFilePath.c_str());
    if (!file.is_open())
    {
        throw ConfigException(FAILED_TO_OPEN_CONFIG_FILE);
    }

    std::string line;
    while (!file.eof())
    {
        std::getline(file, line);
        try
        {
            if (line.find("keepalive_timeout") != std::string::npos)
            {
                parseKeepAliveTimeout(line);
            }
            if (line.find("server {") != std::string::npos)
            {
                parseServer(file);
            }
        }
        catch(const ConfigException& e)
        {
            throw e;
        }
    }
    file.close();
}
void Config::parseKeepAliveTimeout(std::string& line)
{
    std::istringstream iss(line);
    std::string key, value;
    iss >> key;
    getline(iss, value);
    if (key != "keepalive_timeout")
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    if (!isValidValue(value) || !isDigitStr(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    keepalive_timeout = atoi(value.c_str());
    if (keepalive_timeout > MAX_TIMEOUT || keepalive_timeout == 0)
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
}
// 서버 설정 파싱
void Config::parseServer(std::ifstream& file)
{
    ServerConfig serverConfig;
    std::string line;
    std::map<std::string, bool> duplicateCheck;

    // 'server {' 문자열의 끝인 '}'가 나올 때까지 반복
    while (std::getline(file, line) && line.find("}") == std::string::npos)
    {
        if (line.empty())
            continue;
        std::istringstream iss(line);
        std::string key, value;
        iss >> key;
        getline(iss, value);

        if (duplicateCheck[key])
            throw ConfigException(INVALID_SERVER_CONFIG);
        try
        {
            if (key == "host") parseHost(serverConfig, value);
            else if (key == "port") parsePort(serverConfig, value);
            else if (key == "root") parseRoot(serverConfig, value);
            else if (key == "index") parseIndex(serverConfig, value);
            else if (key == "client_max_body_size") parseClientMaxBodySize(serverConfig, value);
            else if (key == "error_page") parseErrorPage(serverConfig, value);
            else if (key == "location") parseLocation(file, serverConfig, value);
            else throw ConfigException(INVALID_SERVER_CONFIG);
        }
        catch(const ConfigException& e)
        {
            throw e;
        }
        if (key != "location" && key != "error_page")
        {
            duplicateCheck[key] = true;
        }
    }
    serverConfigs.push_back(serverConfig);
}
void Config::parseLocation(std::ifstream& file, ServerConfig& serverConfig, std::string& locationPath)
{
    if (!isValidLocationPath(locationPath))
    {
        throw ConfigException(INVALID_LOCATION_CONFIG);
    }
    LocationConfig locationConfig;
    std::string line;
    std::map<std::string, bool> duplicateCheck;

    while (std::getline(file, line) && line.find("}") == std::string::npos)
    {
        if (line.empty())
            continue;
        std::istringstream iss(line);
        std::string key, value;
        iss >> key;
        getline(iss, value);
        if (duplicateCheck[key])
            throw ConfigException(INVALID_LOCATION_CONFIG);
        try
        {
            if (key == "root") parseRoot(locationConfig, value);
            else if (key == "index") parseIndex(locationConfig, value);
            else if (key == "allow_methods") parseAllowMethods(locationConfig, value);
            else if (key == "directory_listing") parseDirectoryListing(locationConfig, value);
            else if (key == "redirect") parseRedirect(locationConfig, value);
            else if (key == "cgi_interpreter") parseCGI(locationConfig, value);
            else if (key == "location")
            {
                trim(value);
                value = locationPath + value;
                parseLocation(file, serverConfig, value);
            }
            else throw ConfigException(INVALID_LOCATION_CONFIG);
        }
        catch(const ConfigException& e)
        {
            throw e;
        }
        duplicateCheck[key] = true;
    }
    serverConfig.locations[locationPath] = locationConfig;
}
void Config::parseHost(ServerConfig& serverConfig, std::string& value)
{
    if (!isValidValue(value))
        throw ConfigException(INVALID_SERVER_CONFIG);
    serverConfig.host = value;
}
void Config::parsePort(ServerConfig& serverConfig, std::string& value)
{
    if (!isValidValue(value) || !isDigitStr(value))
        throw ConfigException(INVALID_SERVER_CONFIG);
    serverConfig.port = atoi(value.c_str());
    if (serverConfig.port > MAX_PORT_NUM || serverConfig.port == 0)
    {
        throw ConfigException(INVALID_SERVER_CONFIG);
    }
}
void Config::parseRoot(ServerConfig& serverConfig, std::string& value)
{
    if (!isValidValue(value))
        throw ConfigException(INVALID_SERVER_CONFIG);
    serverConfig.root = value;
}
void Config::parseIndex(ServerConfig& serverConfig, std::string& value)
{
    if (!isValidValue(value))
        throw ConfigException(INVALID_SERVER_CONFIG);
    serverConfig.index = value;
}
void Config::parseClientMaxBodySize(ServerConfig& serverConfig, std::string& value)
{
    if (!isValidValue(value) || !isDigitStr(value))
        throw ConfigException(INVALID_SERVER_CONFIG);
    serverConfig.client_max_body_size = atoi(value.c_str());
    if (serverConfig.client_max_body_size == 0)
        throw ConfigException(INVALID_SERVER_CONFIG);
}
void Config::parseErrorPage(ServerConfig& serverConfig, std::string& value)
{
    trim(value);
    std::istringstream iss(value);
    std::vector<int> statuses;
    std::string prefix;
    while (getline(iss, prefix, ' '))
    {
        if (iss.eof())
            break;
        if (!isDigitStr(prefix) || prefix.empty())
            throw ConfigException(INVALID_SERVER_CONFIG);
        size_t status = atoi(prefix.c_str());
        if (status < 100 || status > 599)
            throw ConfigException(INVALID_SERVER_CONFIG);
        statuses.push_back(status);
    }
    if (!isValidValue(prefix))
        throw ConfigException(INVALID_SERVER_CONFIG);
    for (std::vector<int>::iterator it = statuses.begin(); it != statuses.end(); ++it)
        serverConfig.error_pages[*it] = prefix;
}
void Config::parseRoot(LocationConfig& locationConfig, std::string& value)
{
    if (!isValidValue(value))
        throw ConfigException(INVALID_LOCATION_CONFIG);
    locationConfig.root = value;
}
void Config::parseIndex(LocationConfig& locationConfig, std::string& value)
{
    if (!isValidValue(value))
        throw ConfigException(INVALID_LOCATION_CONFIG);
    locationConfig.index = value;
}
void Config::parseAllowMethods(LocationConfig& locationConfig, std::string& value)
{
    if (value.back() != ';')
        throw ConfigException(INVALID_LOCATION_CONFIG);
    value.pop_back();
    std::istringstream iss(value);
    std::string method;
    while (iss >> method)
    {
        if (locationConfig.allow_methods[method] == true)
            throw ConfigException(INVALID_LOCATION_CONFIG);
        if (std::find(implementMethods, implementMethods + implementMethodsSize, method) == implementMethods + implementMethodsSize)
            throw ConfigException(INVALID_LOCATION_CONFIG);
        locationConfig.allow_methods[method] = true;
    }
    if (locationConfig.allow_methods.empty())
        throw ConfigException(INVALID_LOCATION_CONFIG);
}
void Config::parseDirectoryListing(LocationConfig& locationConfig, std::string& value)
{
    if (!isValidValue(value))
        throw ConfigException(INVALID_LOCATION_CONFIG);
    if (value == "on")
        locationConfig.directory_listing = true;
    else if (value == "off")
        locationConfig.directory_listing = false;
    else
        throw ConfigException(INVALID_LOCATION_CONFIG);
}
void Config::parseRedirect(LocationConfig& locationConfig, std::string& value)
{
    if (!isValidValue(value))
        throw ConfigException(INVALID_LOCATION_CONFIG);
    locationConfig.redirect = value;
}
void Config::parseCGI(LocationConfig& locationConfig, std::string& value)
{
    if (!isValidValue(value))
        throw ConfigException(INVALID_LOCATION_CONFIG);
    locationConfig.cgi_interpreter = value;
}
bool Config::isValidValue(std::string& value)
{
    trim(value);
    if (value.back() != ';')
    {
        return false;
    }
    value.pop_back();
    if (value.empty())
    {
        return false;
    }
    if (std::find(value.begin(), value.end(), ' ') != value.end())
    {
        return false;
    }
    return true;
}
bool Config::isValidLocationPath(std::string& locationPath)
{
    if (locationPath.back() != '{' || locationPath.find("//") != std::string::npos)
    {
        return false;
    }
    locationPath.pop_back();
    trim(locationPath);
    if (locationPath.empty())
    {
        return false;
    }
    if (find(locationPath.begin(), locationPath.end(), ' ') != locationPath.end())
    {
        return false;
    }
    if (locationPath.back() != '/' && locationPath.front() != '/' && locationPath.front() != '.')
    {
        return false;
    }
    return true;
}
void Config::verifyConfig(void)
{
    if (serverConfigs.empty())
    {
        throw ConfigException(EMPTY_SERVER_CONFIG);
    }
    for (size_t i = 0; i < serverConfigs.size(); i++)
    {
        if (serverConfigs[i].host.empty())
        {
            throw ConfigException(HOST_NOT_EXIST);
        }
        if (serverConfigs[i].port == -1)
        {
            throw ConfigException(PORT_NOT_EXIST);
        }
        for (std::map<std::string, LocationConfig>::iterator locIt = serverConfigs[i].locations.begin(); locIt != serverConfigs[i].locations.end(); ++locIt)
        {
            if (!serverConfigs[i].root.empty() && locIt->CONFIG.root.empty())
            {
                locIt->CONFIG.root = serverConfigs[i].root;
            }
            if (serverConfigs[i].root.empty() && locIt->CONFIG.root.empty())
            {
                throw ConfigException(ROOT_NOT_EXIST);
            }
            if (locIt->CONFIG.index.empty())
            {
                locIt->CONFIG.index = serverConfigs[i].index;
            }
            if (locIt->CONFIG.allow_methods.empty())
            {
                for (size_t j = 0; j < implementMethodsSize; j++)
                {
                    locIt->CONFIG.allow_methods[implementMethods[j]] = true;
                }
            }
            makePrefix(locIt->LOCATION, locIt->CONFIG);
        }
    }
}
void Config::makePrefix(const std::string& loc, LocationConfig& locConf)
{
    if (locConf.root.back() == '/' && loc.front() == '/')
    {
        locConf.root.pop_back();
        locConf.prefix = locConf.root + loc;
    }
    else if (locConf.root.back() != '/' && loc.front() != '/')
    {
        locConf.prefix = locConf.root + "/" + loc;
    }
    else
    {
        locConf.prefix = locConf.root + loc;
    }
}
const std::vector<ServerConfig>& Config::getServerConfigs() const
{
    return serverConfigs;
}
void Config::print(void) const
{
    std::cout << color::FG_WHITE << "Keepalive timeout: " << keepalive_timeout << "\n\n";

    for (size_t i = 0; i < serverConfigs.size(); i++)
    {
        std::cout << color::FG_WHITE << "Server " << i + 1 << ":\n" << color::RESET
        << "host: " << serverConfigs[i].host << '\n'
        << "port: " << serverConfigs[i].port << '\n'
        << "root: " << serverConfigs[i].root << '\n'
        << "index: " << serverConfigs[i].index << '\n'
        << "client_max_body_size: " << serverConfigs[i].client_max_body_size << '\n'
        << "error_pages:\n";
        for (std::map<size_t, std::string>::const_iterator it = serverConfigs[i].error_pages.begin(); it != serverConfigs[i].error_pages.end(); ++it)
        {
            std::cout << "    " << it->first << " " << it->second << '\n';
        }
        for (std::map<std::string, LocationConfig>::const_iterator it = serverConfigs[i].locations.begin(); it != serverConfigs[i].locations.end(); ++it)
        {
            std::cout << color::FG_YELLOW << "locations: " << it->LOCATION << ":" << '\n' << color::RESET
            << "    " << "root: " << it->CONFIG.root << '\n'
            << "    " << "index: " << it->CONFIG.index << '\n'
            << "    " << "allow_methods: ";
            for (std::map<std::string, bool>::const_iterator it2 = it->CONFIG.allow_methods.begin(); it2 != it->CONFIG.allow_methods.end(); ++it2)
            {
                std::cout << it2->first << " ";
            }
            std::cout << "\n    " << "directory_listing: " << (it->CONFIG.directory_listing ? "on" : "off") << '\n'
            << "    " << "redirect: " << it->CONFIG.redirect << '\n'
            << "    " << "cgi_interpreter: " << it->CONFIG.cgi_interpreter << '\n'
            << "    " << "prefix: " << it->CONFIG.prefix << '\n';
        }
        std::cout << std::endl;
    }
}
size_t Config::getKeepAliveTime() const
{
    return keepalive_timeout;
}

bool isWhitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
void trim(std::string &str)
{
    size_t start = 0;
    size_t end = str.size();
    while (start < end && isWhitespace(str[start]))
    {
        ++start;
    }
    while (end > start && isWhitespace(str[end - 1]))
    {
        --end;
    }
    if (start == end)
    {
        str.clear();
    }
    else
    {
        str = str.substr(start, end - start);
    }
}
bool isDigitStr(const std::string &str)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (!isdigit(str[i]))
        {
            return false;
        }
    }
    return true;
}
