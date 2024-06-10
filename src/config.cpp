#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

Config::Config(const std::string &configFile) : configFile(configFile) {}

void Config::parse() {
	std::ifstream file(configFile.c_str());
	if (!file.is_open()) {
		std::cerr << "Failed to open config file: " << configFile << std::endl;
		std::exit(EXIT_FAILURE);
	}

	while (!file.eof()) {
		std::string line;
		std::getline(file, line);
		if (line.find("server {") != std::string::npos) {
			parserServer(file);
		}
	}

	file.close();
}

void Config::parserServer(std::ifstream &file) {
	ServerConfig serverConfig;
	std::string line;

	while
}
