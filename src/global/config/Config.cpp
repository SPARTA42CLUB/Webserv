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

    // 'server {' 문자열의 끝인 '}'가 나올 때까지 반복
    while (std::getline(file, line) && line.find("}") == std::string::npos)
    {
        if (line.empty())
        {
            continue;
        }
        std::istringstream iss(line);
        std::string key, value;
        iss >> key;
        getline(iss, value);
        
        // key에 따라 설정 값 파싱
        try
        {
            if (key == "host") parseHost(serverConfig, value);
            else if (key == "port") parsePort(serverConfig, value);
            else if (key == "root") parseRoot(serverConfig, value);
            else if (key == "client_max_body_size") parseClientMaxBodySize(serverConfig, value);
            else if (key == "error_page") parseErrorPage(serverConfig, value);
            else if (key == "location") parseLocation(file, serverConfig, value);
            else throw ConfigException(INVALID_CONFIG_FILE);
        }
        catch(const ConfigException& e)
        {
            throw e;
        }
    }
    serverConfigs.push_back(serverConfig);
}
void Config::parseLocation(std::ifstream& file, ServerConfig& serverConfig, std::string& locationPath)
{
    if (!isValidLocationPath(locationPath))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    LocationConfig locationConfig;
    std::string line;

    while (std::getline(file, line) && line.find("}") == std::string::npos)
    {
        if (line.empty())
        {
            continue;
        }
        std::istringstream iss(line);
        std::string key, value;
        iss >> key;
        getline(iss, value);

        try
        {
            if (key == "root") parseRoot(locationConfig, value);
            else if (key == "index") parseIndex(locationConfig, value);
            else if (key == "allow_methods") parseAllowMethods(locationConfig, value);
            else if (key == "directory_listing") parseDirectoryListing(locationConfig, value);
            else if (key == "redirect") parseRedirect(locationConfig, value);
            else if (key == "cgi") parseCGI(locationConfig, value);
            else if (key == "location")
            {
                trim(value);
                value = locationPath + value;
                parseLocation(file, serverConfig, value);
            }
            else throw ConfigException(INVALID_CONFIG_FILE);
        }
        catch(const ConfigException& e)
        {
            throw e;
        }
        
    }
    serverConfig.locations[locationPath] = locationConfig;
}
void Config::parseHost(ServerConfig& serverConfig, std::string& value)
{
    if (!isValidValue(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    serverConfig.host = value;
}
void Config::parsePort(ServerConfig& serverConfig, std::string& value)
{
    if (!isValidValue(value) || !isDigitStr(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    serverConfig.port = atoi(value.c_str());
    if (serverConfig.port > MAX_PORT_NUM || serverConfig.port == 0)
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
}
void Config::parseRoot(ServerConfig& serverConfig, std::string& value)
{
    if (!isValidValue(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    serverConfig.root = value;
}
void Config::parseClientMaxBodySize(ServerConfig& serverConfig, std::string& value)
{
    if (!isValidValue(value) || !isDigitStr(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    serverConfig.client_max_body_size = atoi(value.c_str());
    if (serverConfig.client_max_body_size == 0)
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
}
void Config::parseErrorPage(ServerConfig& serverConfig, std::string& value)
{
    trim(value);
    std::istringstream iss(value);
    std::vector<int> statuses;
    std::string path;
    while (getline(iss, path, ' '))
    {
        if (iss.eof())
        {
            break;
        }
        if (!isDigitStr(path) || path.empty())
        {
            throw ConfigException(INVALID_CONFIG_FILE);
        }
        size_t status = atoi(path.c_str());
        if (status < 100 || status > 599)
        {
            throw ConfigException(INVALID_CONFIG_FILE);
        }
        statuses.push_back(status);
    }
    if (!isValidValue(path))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    for (std::vector<int>::iterator it = statuses.begin(); it != statuses.end(); ++it)
    {
        serverConfig.error_pages[*it] = path;
    }
}
void Config::parseRoot(LocationConfig& locationConfig, std::string& value)
{
    if (!isValidValue(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    locationConfig.root = value;
}
void Config::parseIndex(LocationConfig& locationConfig, std::string& value)
{
    if (!isValidValue(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    locationConfig.index = value;
}
void Config::parseAllowMethods(LocationConfig& locationConfig, std::string& value)
{
    if (value.back() != ';')
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    value.pop_back();
    trim(value);
    std::istringstream iss(value);
    std::string method;
    while (getline(iss, method, ' '))
    {
        if (iss.eof())
        {
            break;
        }
        if (method != "GET" && method != "HEAD" && method != "POST" && method != "DELETE")
        {
            throw ConfigException(INVALID_CONFIG_FILE);
        }
        locationConfig.allow_methods.push_back(method);
    }
}
void Config::parseDirectoryListing(LocationConfig& locationConfig, std::string& value)
{
    if (!isValidValue(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    if (value == "on")
    {
        locationConfig.directory_listing = true;
    }
    else if (value == "off")
    {
        locationConfig.directory_listing = false;
    }
    else
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
}
void Config::parseRedirect(LocationConfig& locationConfig, std::string& value)
{
    if (!isValidValue(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    locationConfig.redirect = value;
}
void Config::parseCGI(LocationConfig& locationConfig, std::string& value)
{
    if (!isValidValue(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    locationConfig.cgi = value;
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
        for (std::map<std::string, LocationConfig>::iterator it = serverConfigs[i].locations.begin(); it != serverConfigs[i].locations.end(); ++it)
        {
            if (!serverConfigs[i].root.empty() && it->second.root.empty())
            {
                it->second.root = serverConfigs[i].root;
            }
            if (serverConfigs[i].root.empty() && it->second.root.empty())
            {
                throw ConfigException(ROOT_NOT_EXIST);
            }
            makePath(it->first, it->second);
        }
    }
}
void Config::makePath(const std::string& loc, LocationConfig& locConf)
{
    if (locConf.root.back() == '/' && loc.front() == '/')
    {
        locConf.root.pop_back();
        locConf.path = locConf.root + loc;
    }
    else if (locConf.root.back() != '/' && loc.front() != '/')
    {
        locConf.path = locConf.root + "/" + loc;
    }
    else
    {
        locConf.path = locConf.root + loc;
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
        << "client_max_body_size: " << serverConfigs[i].client_max_body_size << '\n'
        << "error_pages:\n";
        for (std::map<size_t, std::string>::const_iterator it = serverConfigs[i].error_pages.begin(); it != serverConfigs[i].error_pages.end(); ++it)
        {
            std::cout << "    " << it->first << " " << it->second << '\n';
        }
        for (std::map<std::string, LocationConfig>::const_iterator it = serverConfigs[i].locations.begin(); it != serverConfigs[i].locations.end(); ++it)
        {
            std::cout << color::FG_YELLOW << "locations: " << it->first << ":" << '\n' << color::RESET
            << "    " << "root: " << it->second.root << '\n'
            << "    " << "index: " << it->second.index << '\n'
            << "    " << "allow_methods: ";
            for (size_t j = 0; j < it->second.allow_methods.size(); j++)
            {
                std::cout << it->second.allow_methods[j] << " ";
            }
            std::cout << "\n    " << "directory_listing: " << (it->second.directory_listing ? "on" : "off") << '\n'
            << "    " << "redirect: " << it->second.redirect << '\n'
            << "    " << "cgi: " << it->second.cgi << '\n'
            << "    " << "path: " << it->second.path << '\n';
        }
        std::cout << std::endl;
    }
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
