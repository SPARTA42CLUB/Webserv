#include "Config.hpp"
#include "error.hpp"
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <iostream>

Config::Config(const std::string &configFile) : configFile(configFile) {}

// Config 파일 파싱
void Config::parse() {
	std::ifstream file(configFile.c_str());
	if (!file.is_open())
		exit_with_error("Failed to open config file");

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
		} else if (key == "location") {
			std::string location;
			iss >> location;
			parseLocation(file, serverConfig, location);
		}
	}

	// 서버 설정 벡터에 추가
	servers.push_back(serverConfig);
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
			iss >> locationConfig.root;
		else if (key == "index")
			iss >> locationConfig.index;
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
	}

	// Location 설정 추가
	serverConfig.locations[locationPath] = locationConfig;
}

// 서버 설정 벡터 반환
std::vector<ServerConfig> Config::getServers() const {
	return servers;
}
