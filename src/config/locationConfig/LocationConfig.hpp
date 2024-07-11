#ifndef LOCATION_CONFIG_HPP
#define LOCATION_CONFIG_HPP

#include <map>
#include <string>

#define LOCATION first
#define CONFIG   second

const size_t DEFAULT_TIMEOUT = 60;
const size_t MAX_TIMEOUT = 600;
const size_t DEFAULT_MAX_BODY_SIZE = 1024;  // 1KB
const int MAX_PORT_NUM = 65535;

class LocationConfig
{
public:
    std::string root;
    std::string alias;
    std::string index;
    std::map<std::string, bool> allow_methods;
    bool directory_listing;
    std::string redirect;
    std::string cgi_interpreter;
    std::string proxy_pass;

    static const size_t implementMethodsSize;
    static const std::string implementMethods[];

    LocationConfig();
    ~LocationConfig();
    LocationConfig(const LocationConfig& rhs);
    LocationConfig& operator=(const LocationConfig& rhs);
    void parseRoot(std::string& value);
    void parseAlias(std::string& value);
    void parseIndex(std::string& value);
    void parseAllowMethods(std::string& value);
    void parseDirectoryListing(std::string& value);
    void parseRedirect(std::string& value);
    void parseCGI(std::string& value);
    void parseProxyPass(std::string& value);
};

#endif
