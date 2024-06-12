#include "Config.hpp"
#include "Server.hpp"
#include "error.hpp"

int main(int argc, char* argv[]) {
	// Config 기본 경로 설정
	std::string configPath = "config/server.conf";

	// Config 파일 경로 설정
	if (argc > 1) {
		configPath = argv[1];
	}

	// Config 파일 파싱
	Config config(configPath);
	config.parse();

	// Config 파일에서 서버 설정 가져오기
	std::vector<ServerConfig> serverConfigs = config.getServers();
	Server server(serverConfigs);

	// 서버 실행
	try {
		server.run();
	} catch (const std::exception& e) {
		// 서버 실행 중 에러 발생 시 종료
		exit_with_error("Server run failed");
	}

	return 0;
}
