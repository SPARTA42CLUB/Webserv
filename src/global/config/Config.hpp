#pragma once

#include <string>
#include <vector>
#include <map>

struct LocationConfig {
	std::string root;
	std::string index;
	std::vector<std::string> allow_methods;
	std::string upload_dir;
	bool directory_listing;
	std::string redirect;
	std::string cgi;
    std::string proxy_pass;

	LocationConfig() : directory_listing(false) {}
};

struct ServerConfig {
	std::string server_name;
	std::string host;
	int port;
	std::map<int, std::string> error_pages;
	size_t client_max_body_size;
	std::map<std::string, LocationConfig> locations;
	int keepalive_timeout;

	ServerConfig() : port(80), client_max_body_size(0), keepalive_timeout(60) {}
};

class Config {
public:
	Config(const std::string& configFilePath);
	std::vector<ServerConfig> getServerConfigs() const;
	const ServerConfig& getServerConfig(const std::string& serverName, int port) const;

private:
	void parse();
	std::string configFilePath;
	std::vector<ServerConfig> serverConfigs;

	void parseServer(std::ifstream &file);
	void parseLocation(std::ifstream &file, ServerConfig &serverConfig, const std::string &locationPath);
};
