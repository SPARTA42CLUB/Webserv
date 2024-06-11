#include "Config.hpp"
#include "Server.hpp"
#include "error.hpp"

int main(int argc, char* argv[]) {

	std::string configPath = "config/server.conf";

	if (argc > 1) {
		configPath = argv[1];
	}

	Config config(configPath);
	config.parse();

	std::vector<ServerConfig> serverConfigs = config.getServers();
	Server server(serverConfigs);

	try {
		server.run();
	} catch (const std::exception& e) {
		exit_with_error("Server run failed");
	}

	return 0;
}
