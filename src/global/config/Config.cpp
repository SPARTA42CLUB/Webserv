#include "Config.hpp"
#include "Exception.hpp"
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <iostream>

Config::Config(const std::string& configFilePath) : configFilePath(configFilePath) {
	parse();
}

// Config 파일 파싱
void Config::parse() {
	std::ifstream file(configFilePath.c_str());
	if (!file.is_open())
    {
        throw Exception(FAILED_TO_OPEN_CONFIG_FILE);
    }

	// 파일 끝까지 반복
	while (!file.eof()) {
		std::string line;
		std::getline(file, line);
		// server { 문자열이 있으면 서버 설정 파싱
		if (line.find("server {") != std::string::npos) {
			parseServer(file);
		}
	}

	file.close();
}

// 서버 설정 파싱
void Config::parseServer(std::ifstream &file) {
	ServerConfig serverConfig;
	std::string line;

	// 'server {' 문자열의 끝인 '}'가 나올 때까지 반복
	while (std::getline(file, line) && line.find("}") == std::string::npos) {
		std::istringstream iss(line);
		std::string key;
		iss >> key;

		// key에 따라 설정 값 파싱
		if (key == "host")
			iss >> serverConfig.host;
		else if (key == "port")
			iss >> serverConfig.port;
		else if (key == "server_name")
			iss >> serverConfig.server_name;
		else if (key == "error_page") {
			int code;
			std::string path;
			iss >> code >> path;
			serverConfig.error_pages[code] = path;
		} else if (key == "client_max_body_size") {
			std::string size;
			iss >> size;
			serverConfig.client_max_body_size = strtoul(size.c_str(), NULL, 10);
		} else if (key == "keepalive_timeout") {
			iss >> serverConfig.keepalive_timeout;
		} else if (key == "location") {
			std::string location;
			iss >> location;
			parseLocation(file, serverConfig, location);
		}
	}

	// 서버 설정 벡터에 추가
	serverConfigs.push_back(serverConfig);
}

// Location 설정 파싱
void Config::parseLocation(std::ifstream& file, ServerConfig& serverConfig, const std::string &locationPath) {
	LocationConfig locationConfig;
	std::string line;

	// 'location {' 문자열의 끝인 '}'가 나올 때까지 반복
	while (std::getline(file, line) && line.find("}") == std::string::npos) {
		std::istringstream iss(line);
		std::string key;
		iss >> key;

		// key에 따라 설정 값 파싱
		if (key == "root")
        {
			iss >> locationConfig.root;
            if (locationConfig.root.back()== ';')
            {
                locationConfig.root.pop_back();
            }
        }
		else if (key == "index")
        {
            iss >> locationConfig.index;
            if (locationConfig.index.back()== ';')
            {
                locationConfig.index.pop_back();
            }
        }
		else if (key == "allow_methods") {
			std::string method;
			while (iss >> method)
				locationConfig.allow_methods.push_back(method);
		} else if (key == "upload_dir")
			iss >> locationConfig.upload_dir;
		else if (key == "redirect")
			iss >> locationConfig.redirect;
		else if (key == "directory_listing") {
			std::string value;
			iss >> value;
			locationConfig.directory_listing = (value == "on");
		} else if (key == "cgi")
			iss >> locationConfig.cgi;
        else if (key == "proxy_pass")
        {
            iss >> locationConfig.proxy_pass;
            if (locationConfig.proxy_pass.back()== ';')
            {
                locationConfig.proxy_pass.pop_back();
            }
        }
	}

	// Location 설정 추가
	serverConfig.locations[locationPath] = locationConfig;
}

// 서버 설정 벡터 반환
std::vector<ServerConfig> Config::getServerConfigs() const {
	return serverConfigs;
}

const ServerConfig& Config::getServerConfig(const std::string& serverName, int port) const {
	for (size_t i = 0; i < serverConfigs.size(); ++i) {
		const ServerConfig& serverConfig = serverConfigs[i];
		if (serverConfig.port == port && serverConfig.server_name == serverName)
			return serverConfig;
	}

	throw std::runtime_error("No server configuration found for host " + serverName + " and port " + std::to_string(port));
}
