#pragma once

#include <string>

void exit_with_error(const std::string& msg);
void exit_with_error_close(const std::string& msg, int socket);
