#pragma once

#include <map>
#include <string>
#include <vector>

struct LocationConfig
{
    std::string root;
    std::string index;
    std::vector<std::string> allow_methods;
    std::string upload_dir;
    bool directory_listing;
    std::string redirect;
    std::string cgi;

    LocationConfig()
    : directory_listing(false)
    {
    }
};

struct ServerConfig
{
    std::string host;
    size_t port;
    std::string server_name;
    size_t keepalive_timeout;
    size_t client_max_body_size;
    std::map<size_t, std::string> error_pages;
    std::map<std::string, LocationConfig> locations;

    ServerConfig()
    : port(80)
    , keepalive_timeout(60)
    , client_max_body_size(0)
    {
    }
};

class Config
{
private:
    std::string configFilePath;
    std::vector<ServerConfig> serverConfigs;
    void parse();
    bool isValidValue(std::string &value);
    bool isValidLocationPath(std::string &locationPath);

    void parseServer(std::ifstream &file);
    // Parse ServerConfig
    void parseHost(ServerConfig& serverConfig, std::string& value);
    void parsePort(ServerConfig& serverConfig, std::string& value);
    void parseServerName(ServerConfig& serverConfig, std::string& value);
    void parseKeepAliveTimeout(ServerConfig& serverConfig, std::string& value);
    void parseClientMaxBodySize(ServerConfig& serverConfig, std::string& value);
    void parseErrorPage(ServerConfig& serverConfig, std::string& value);
    void parseLocation(std::ifstream &file, ServerConfig &serverConfig, std::string& locationPath);
    // Parse LocationConfig
    void parseRoot(LocationConfig& locationConfig, std::string& value);
    void parseIndex(LocationConfig& locationConfig, std::string& value);
    void parseAllowMethods(LocationConfig& locationConfig, std::string& value);
    void parseUploadDir(LocationConfig& locationConfig, std::string& value);
    void parseDirectoryListing(LocationConfig& locationConfig, std::string& value);
    void parseRedirect(LocationConfig& locationConfig, std::string& value);
    void parseCgi(LocationConfig& locationConfig, std::string& value);

    const ServerConfig& getDefaultServerConfig() const;

public:
    Config(const std::string &configFilePath);
    const std::vector<ServerConfig>& getServerConfigs() const;
    const ServerConfig& getServerConfigByHost(std::string host) const;

    // TEST: 파싱 결과 출력
    void print(void);
};
