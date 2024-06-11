#include <errno.h>
#include <sys/socket.h>
#include <iostream>

int main(int argc, char const *argv[])
{
    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("webserver (socket)");
        return 1;
    }
    std::cout << "socket created successfully\n";

    return 0;
}
