#include "error.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>

// 에러 메시지 출력 후 종료
void exit_with_error(const std::string& msg) {
	std::cerr << msg << ": " << strerror(errno) << std::endl;
	std::exit(EXIT_FAILURE);
}

// 에러 메시지 출력 후 종료 및 소켓 닫기
void exit_with_error_close(const std::string& msg, int socket) {
	std::cerr << msg << ": " << strerror(errno) << std::endl;
	close(socket);
	std::exit(EXIT_FAILURE);
}
