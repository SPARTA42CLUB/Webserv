#include "Config.hpp"
#include <iostream>
#include <sstream>
#include "ConfigException.hpp"
#include "color.hpp"
#include "parse.hpp"

Config::Config(const std::string& configFilePath)
: keepalive_timeout(DEFAULT_TIMEOUT)
, cgi_timeout(DEFAULT_TIMEOUT > 10 ? 5 : DEFAULT_TIMEOUT / 2)
, serverConfigs()
{
    try
    {
        parse(configFilePath);
        verifyConfig();
        cgi_timeout = keepalive_timeout > 10 ? 5 : keepalive_timeout / 2;
    }
    catch (const ConfigException& e)
    {
        throw e;
    }
}
Config::~Config()
{
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
                parseKeepAliveTimeout(line);
            if (line.find("server {") != std::string::npos)
                parseServer(file);
        }
        catch (const ConfigException& e)
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
        throw ConfigException(INVALID_CONFIG_FILE);
    if (!isValidValue(value) || !isDigitStr(value))
        throw ConfigException(INVALID_KEEPALIVE_TIMEOUT);
    keepalive_timeout = atoi(value.c_str());
    if (keepalive_timeout > MAX_TIMEOUT || keepalive_timeout == 0)
        throw ConfigException(INVALID_KEEPALIVE_TIMEOUT);
}
// 서버 설정 파싱
void Config::parseServer(std::ifstream& file)
{
    serverConfigs.push_back(ServerConfig());
    ServerConfig& serverConfig = serverConfigs.back();
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
            if (key == "host")
                serverConfig.parseHost(value);
            else if (key == "port")
                serverConfig.parsePort(value);
            else if (key == "root")
                serverConfig.parseRoot(value);
            else if (key == "index")
                serverConfig.parseIndex(value);
            else if (key == "client_max_body_size")
                serverConfig.parseClientMaxBodySize(value);
            else if (key == "error_page")
                serverConfig.parseErrorPage(value);
            else if (key == "location")
            {
                if (!serverConfig.isValidLocationPath(value))
                    throw ConfigException(INVALID_LOCATION_CONFIG);
                serverConfig.parseLocation(file, value);
            }
            else
                throw ConfigException(INVALID_SERVER_CONFIG);
        }
        catch (const ConfigException& e)
        {
            throw e;
        }
        if (key != "location" && key != "error_page")
            duplicateCheck[key] = true;
    }
}
void Config::verifyConfig(void)
{
    std::map<int, bool> duplicateCheck;
    if (serverConfigs.empty())
        throw ConfigException(EMPTY_SERVER_CONFIG);
    for (std::vector<ServerConfig>::iterator servIt = serverConfigs.begin(); servIt != serverConfigs.end(); ++servIt)
    {
        if (servIt->host.empty())
            throw ConfigException(HOST_NOT_EXIST);
        if (servIt->port == -1)
            throw ConfigException(PORT_NOT_EXIST);
        if (servIt->locations.empty())
            throw ConfigException(LOCATION_NOT_EXIST);
        for (std::map<std::string, LocationConfig>::iterator locIt = servIt->locations.begin(); locIt != servIt->locations.end(); ++locIt)
        {
            if (!servIt->root.empty() && locIt->CONFIG.root.empty() && locIt->CONFIG.alias.empty())
                locIt->CONFIG.root = servIt->root;
            if (!servIt->index.empty() && locIt->CONFIG.index.empty())
                locIt->CONFIG.index = servIt->index;
            if (locIt->CONFIG.root.empty() && locIt->CONFIG.alias.empty() && locIt->CONFIG.redirect.empty() && locIt->CONFIG.proxy_pass.empty())
                throw ConfigException(ROOT_NOT_EXIST);
            if (!locIt->CONFIG.root.empty() && !locIt->CONFIG.alias.empty())
                throw ConfigException(DUPLICATE_ROOT_ALIAS);
            // root나 alias 뒤에 붙는 target uri나 location은 '/'로 시작하므로 '/'를 제거
            if (locIt->CONFIG.root.back() == '/')
                pop_back(locIt->CONFIG.root);
            if (locIt->CONFIG.alias.back() == '/')
                pop_back(locIt->CONFIG.alias);
            if (locIt->CONFIG.allow_methods.empty())
            {
                for (size_t i = 0; i < LocationConfig::implementMethodsSize; i++)
                    locIt->CONFIG.allow_methods[LocationConfig::implementMethods[i]] = true;
            }
        }
        if (duplicateCheck[servIt->port])
            throw ConfigException(DUPLICATE_PORT);
        duplicateCheck[servIt->port] = true;
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
        std::cout << color::FG_WHITE << "Server " << i + 1 << ":\n"
                  << color::RESET << "host: " << serverConfigs[i].host << '\n'
                  << "port: " << serverConfigs[i].port << '\n'
                  << "root: " << serverConfigs[i].root << '\n'
                  << "index: " << serverConfigs[i].index << '\n'
                  << "client_max_body_size: " << serverConfigs[i].client_max_body_size << '\n'
                  << color::FG_RED << "error_pages:\n" << color::RESET;
        for (std::map<size_t, std::string>::const_iterator it = serverConfigs[i].error_pages.begin(); it != serverConfigs[i].error_pages.end(); ++it)
        {
            std::cout << "    " << it->first << " " << it->second << '\n';
        }
        for (std::map<std::string, LocationConfig>::const_iterator it = serverConfigs[i].locations.begin(); it != serverConfigs[i].locations.end(); ++it)
        {
            std::cout << color::FG_YELLOW << "locations: " << it->LOCATION << ":" << '\n'
                      << color::RESET << "    "
                      << "root: " << it->CONFIG.root << "\n    "
                      << "alias: " << it->CONFIG.alias << "\n    "
                      << "index: " << it->CONFIG.index << "\n    "
                      << "allow_methods: ";
            for (std::map<std::string, bool>::const_iterator it2 = it->CONFIG.allow_methods.begin(); it2 != it->CONFIG.allow_methods.end(); ++it2)
            {
                std::cout << it2->first << ' ';
            }
            std::cout << "\n    "
                      << "directory_listing: " << (it->CONFIG.directory_listing ? "on" : "off") << "\n    "
                      << "redirect: " << it->CONFIG.redirect << "\n    "
                      << "cgi_interpreter: " << it->CONFIG.cgi_interpreter << "\n    "
                      << "proxy_pass: " << it->CONFIG.proxy_pass << '\n';
        }
        std::cout << std::endl;
    }
}
size_t Config::getKeepAliveTime() const
{
    return keepalive_timeout;
}
size_t Config::getCgiTimeout() const
{
    return cgi_timeout;
}
