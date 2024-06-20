#include "Config.hpp"
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "ConfigException.hpp"

Config::Config(const std::string& configFilePath)
: configFilePath(configFilePath)
{
    try
    {
        parse();
    }
    catch(const ConfigException& e)
    {
        throw e;
    }
    
}

// Config 파일 파싱
void Config::parse()
{
    std::ifstream file(configFilePath.c_str());
    if (!file.is_open())
    {
        throw ConfigException(FAILED_TO_OPEN_CONFIG_FILE);
    }

    // 파일 끝까지 반복
    while (!file.eof())
    {
        std::string line;
        std::getline(file, line);
        // server { 문자열이 있으면 서버 설정 파싱
        if (line.find("server {") != std::string::npos)
        {
            try
            {
                parseServer(file);
            }
            catch(const ConfigException& e)
            {
                throw e;
            }
        }
    }

    file.close();
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
            else if (key == "server_name") parseServerName(serverConfig, value);
            else if (key == "keepalive_timeout") parseKeepAliveTimeout(serverConfig, value);
            else if (key == "client_max_body_size") parseClientMaxBodySize(serverConfig, value);
            else if (key == "error_page") parseErrorPage(serverConfig, value);
            else if (key == "location") parseLocation(file, serverConfig, value);
        }
        catch(const ConfigException& e)
        {
            throw e;
        }
        
    }

    // 서버 설정 벡터에 추가
    serverConfigs.push_back(serverConfig);
}

// Location 설정 파싱
void Config::parseLocation(std::ifstream& file, ServerConfig& serverConfig, std::string& locationPath)
{
    if (locationPath.back() == '{')
    {
        locationPath.pop_back();
    }
    // 앞 뒤 공백 제거
    if (find(locationPath.begin(), locationPath.end(), ' ') == locationPath.begin())
    {
        locationPath.erase(locationPath.begin());
    }
    if (find(locationPath.begin(), locationPath.end(), ' ') == locationPath.end() - 1)
    {
        locationPath.pop_back();
    }
    if (!isValidLocationPath(locationPath))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    // Remove trailing whitespace from locationPath
    LocationConfig locationConfig;
    std::string line;

    // 'location {' 문자열의 끝인 '}'가 나올 때까지 반복
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
        if (key == "root") parseRoot(locationConfig, value);
        else if (key == "index") parseIndex(locationConfig, value);
        else if (key == "allow_methods") parseAllowMethods(locationConfig, value);
        else if (key == "upload_dir") parseUploadDir(locationConfig, value);
        else if (key == "directory_listing") parseDirectoryListing(locationConfig, value);
        else if (key == "redirect") parseRedirect(locationConfig, value);
        else if (key == "cgi") parseCgi(locationConfig, value);
        // TODO: Recursive location block parsing
        // else if (key == "location") parseLocation(file, serverConfig, value);
    }

    // Location 설정 추가
    serverConfig.locations[locationPath] = locationConfig;
}
// 서버 설정 벡터 반환
const std::vector<ServerConfig>& Config::getServerConfigs() const
{
    return serverConfigs;
}
bool Config::isValidValue(std::string& value)
{
    if (value.front() == ' ')
    {
        value.erase(value.begin());
    }
    if (value.back() == ';')
    {
        value.pop_back();
    }
    else 
    {
        return false;
    }
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
    if (locationPath.front() == ' ')
    {
        locationPath.erase(locationPath.begin());
    }
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
    if (!isValidValue(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    serverConfig.port = atoi(value.c_str());
    if (serverConfig.port > 65535)
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
}
void Config::parseServerName(ServerConfig& serverConfig, std::string& value)
{
    if (!isValidValue(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    serverConfig.server_name = value;
}
void Config::parseKeepAliveTimeout(ServerConfig& serverConfig, std::string& value)
{
    if (!isValidValue(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    serverConfig.keepalive_timeout = atoi(value.c_str());
    if (serverConfig.keepalive_timeout == 0)
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
}
void Config::parseClientMaxBodySize(ServerConfig& serverConfig, std::string& value)
{
    if (!isValidValue(value))
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
    if (value.front() == ' ')
    {
        value.erase(value.begin());
    }
    std::istringstream iss(value);
    std::vector<int> statuses;
    std::string path;
    while (getline(iss, path, ' '))
    {
        if (iss.eof())
        {
            break;
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
    if (value.back() == ';')
    {
        value.pop_back();
    }
    if (value.front() == ' ')
    {
        value.erase(value.begin());
    }
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

void Config::parseUploadDir(LocationConfig& locationConfig, std::string& value)
{
    if (!isValidValue(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    locationConfig.upload_dir = value;
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
void Config::parseCgi(LocationConfig& locationConfig, std::string& value)
{
    if (!isValidValue(value))
    {
        throw ConfigException(INVALID_CONFIG_FILE);
    }
    locationConfig.cgi = value;
}

// TEST: 파싱 결과 출력
using namespace std;
void Config::print()
{
    cout << "Config file path: " << configFilePath << endl;

    for (size_t i = 0; i < serverConfigs.size(); i++)
    {
        cout << "Server " << i + 1 << ":" << endl;
        cout << "host: " << serverConfigs[i].host << endl;
        cout << "port: " << serverConfigs[i].port << endl;
        cout << "server_name: " << serverConfigs[i].server_name << endl;
        cout << "keepalive_timeout: " << serverConfigs[i].keepalive_timeout << endl;
        cout << "client_max_body_size: " << serverConfigs[i].client_max_body_size << endl;
        cout << "error_pages: " << endl;
        for (std::map<size_t, std::string>::iterator it = serverConfigs[i].error_pages.begin(); it != serverConfigs[i].error_pages.end(); ++it)
        {
            cout << "    " << it->first << " " << it->second << endl;
        }
        for (std::map<std::string, LocationConfig>::iterator it = serverConfigs[i].locations.begin(); it != serverConfigs[i].locations.end(); ++it)
        {
            cout << "locations: " << it->first << ":" << endl;
            cout << "    " << "root: " << it->second.root << endl;
            cout << "    " << "index: " << it->second.index << endl;
            cout << "    " << "allow_methods: ";
            for (size_t j = 0; j < it->second.allow_methods.size(); j++)
            {
                cout << "    " << it->second.allow_methods[j] << " ";
            }
            cout << endl;
            cout << "    " << "upload_dir: " << it->second.upload_dir << endl;
            cout << "    " << "directory_listing: " << it->second.directory_listing << endl;
            cout << "    " << "redirect: " << it->second.redirect << endl;
            cout << "    " << "cgi: " << it->second.cgi << endl;
        }
    }
    exit(0);
}
