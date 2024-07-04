#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <fstream>
#include "LocationConfig.hpp"

class ServerConfig
{
public:
    std::string host;
    int port;
    std::string root;
    std::string index;
    size_t client_max_body_size;
    std::map<size_t, std::string> error_pages;
    std::map<std::string, LocationConfig> locations;

    ServerConfig();
    ~ServerConfig();
    ServerConfig(const ServerConfig& rhs);
    ServerConfig& operator=(const ServerConfig& rhs);
    void parseHost(std::string& value);
    void parsePort(std::string& value);
    void parseRoot(std::string& value);
    void parseIndex(std::string& value);
    void parseClientMaxBodySize(std::string& value);
    void parseErrorPage(std::string& value);
    void parseLocation(std::ifstream& file, std::string& locationPath, LocationConfig* parentsLocation = NULL);

    bool isValidLocationPath(std::string& locationPath);
};

#endif
