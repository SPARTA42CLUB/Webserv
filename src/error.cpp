#include "error.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>

void exit_with_error(const std::string& msg) {
	std::cerr << msg << ": " << strerror(errno) << std::endl;
	std::exit(EXIT_FAILURE);
}

void exit_with_error_close(const std::string& msg, int socket) {
	std::cerr << msg << ": " << strerror(errno) << std::endl;
	close(socket);
	std::exit(EXIT_FAILURE);
}
