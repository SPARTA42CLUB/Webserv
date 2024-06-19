#include <iostream>
#include "test.hpp"
#include <iomanip>
#include <ctime>
#include <sstream>

int main(void)
{
    std::cout << "Hello, World!" << std::endl;

    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    std::ostringstream oss;
    // Date: Wed, 19 Jun 2024 05:11:12 GMT
    oss << std::put_time(timeinfo, "%a, %d %b %Y %H:%M:%S GMT");
    std::string date = oss.str();
    return 0;
}
