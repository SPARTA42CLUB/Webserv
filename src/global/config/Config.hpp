#pragma once

#include <map>
#include <string>
#include <vector>

const size_t DEFAULT_TIMEOUT = 60;
const size_t MAX_TIMEOUT = 600;
const size_t DEFAULT_MAX_BODY_SIZE = 1024;  // 1KB
const int MAX_PORT_NUM = 65535;

struct LocationConfig
{
    std::string root;
    std::string index;
    std::vector<std::string> allow_methods;
    bool directory_listing;
    std::string redirect;
    std::string cgi;
    std::string path;

    LocationConfig()
    : root()
    , index()
    , allow_methods()
    , directory_listing(false)
    , redirect()
    , cgi()
    , path()
    {
    }
};

struct ServerConfig
{
    std::string host;
    int port;
    std::string root;
    size_t client_max_body_size;
    std::map<size_t, std::string> error_pages;
    std::map<std::string, LocationConfig> locations;

    ServerConfig()
    : host()
    , port(-1)
    , root()
    , client_max_body_size(DEFAULT_MAX_BODY_SIZE)
    , error_pages()
    , locations()
    {
    }
};

class Config
{
private:
    size_t keepalive_timeout;
    std::vector<ServerConfig> serverConfigs;

    void parse(const std::string& configFilePath);
    // Parse Config
    void parseKeepAliveTimeout(std::string& line);
    void parseServer(std::ifstream& file);
    // Parse ServerConfig
    void parseHost(ServerConfig& serverConfig, std::string& value);
    void parsePort(ServerConfig& serverConfig, std::string& value);
    void parseRoot(ServerConfig& serverConfig, std::string& value);
    void parseClientMaxBodySize(ServerConfig& serverConfig, std::string& value);
    void parseErrorPage(ServerConfig& serverConfig, std::string& value);
    void parseLocation(std::ifstream& file, ServerConfig& serverConfig, std::string& locationPath);
    // Parse LocationConfig
    void parseRoot(LocationConfig& locationConfig, std::string& value);
    void parseIndex(LocationConfig& locationConfig, std::string& value);
    void parseAllowMethods(LocationConfig& locationConfig, std::string& value);
    void parseDirectoryListing(LocationConfig& locationConfig, std::string& value);
    void parseRedirect(LocationConfig& locationConfig, std::string& value);
    void parseCGI(LocationConfig& locationConfig, std::string& value);
    // Check Validity
    bool isValidValue(std::string& value);
    bool isValidLocationPath(std::string& locationPath);

    void verifyConfig(void);
    void makePath(const std::string& loc, LocationConfig& locConf);

    const ServerConfig& getDefaultServerConfig() const;

public:
    Config(const std::string& configFilePath);
    const std::vector<ServerConfig>& getServerConfigs() const;
    const ServerConfig& getServerConfigByHost(std::string host) const;

    // TEST: 파싱 결과 출력
    void print(void) const;
};
