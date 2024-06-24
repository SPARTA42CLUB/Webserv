#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

void sendChunk(int sockfd, const std::string& chunkSize, const std::string& chunkData) {
    std::string chunk = chunkSize + "\r\n" + chunkData + "\r\n";
    int n = send(sockfd, chunk.c_str(), chunk.length(), 0);
    if (n < 0) {
        std::cerr << "ERROR writing to socket" << std::endl;
    }
}

void sendLastChunk(int sockfd) {
    std::string lastChunk = "0\r\n\r\n";
    int n = write(sockfd, lastChunk.c_str(), lastChunk.length());
    if (n < 0) {
        std::cerr << "ERROR writing last chunk to socket" << std::endl;
    }
}

void sendChunkedData(const std::string& host, int port, const std::string& path) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "ERROR opening socket" << std::endl;
        return;
    }

    server = gethostbyname(host.c_str());
    if (server == nullptr) {
        std::cerr << "ERROR, no such host" << std::endl;
        close(sockfd);
        return;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "ERROR connecting" << std::endl;
        close(sockfd);
        return;
    }

    // HTTP 헤더 전송
    std::string headers =
        "POST " + path + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n";
    int n = send(sockfd, headers.c_str(), headers.length(), 0);
    if (n < 0) {
        std::cerr << "ERROR writing headers to socket" << std::endl;
        close(sockfd);
        return;
    }

    // 청크 데이터 전송
    sendChunk(sockfd, "4", "Wiki");
    sendChunk(sockfd, "5", "pedia");
    sendChunk(sockfd, "E", " in\r\n\r\nchunks.");

    // 마지막 청크 전송
    sendLastChunk(sockfd);

    close(sockfd);
}

int main() {
    std::string host = "localhost";
    int port = 8081;
    std::string path = "/";

    sendChunkedData(host, port, path);

    return 0;
}
