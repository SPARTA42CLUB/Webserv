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
};

struct ServerConfig {
	std::string host;
	int port;
	std::string server_name;
	std::map<int, std::string> error_pages;
	size_t client_max_body_size;
	std::map<std::string, LocationConfig> locations;
};

class Config {
	public:
	Config(const std::string& configFile);
	void parse();
	std::vector<ServerConfig> getServers() const;

	private:
	std::string configFile;
	std::vector<ServerConfig> servers;

	void parseServer(std::ifstream &file);
	void parseLocation(std::ifstream &file, ServerConfig &serverConfig, const std::string &locationPath);
};
