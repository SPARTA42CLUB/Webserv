#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <vector>
#include "Config.hpp"
#include "Connection.hpp"

class Server
{
private:
    const Config& config;
    std::vector<int> serverSockets;
    std::map<int, Connection*> connectionsMap;

    void setupServerSockets();
    int createServerSocket(ServerConfig serverConfig);
    void setNonBlocking(int socket);
    void checkKeepAlive();

    void acceptClient(int serverSocket);
    void handleClientReadEvent(struct kevent& event);
    void handleClientWriteEvent(struct kevent& event);

    void closeConnection(int socket);
    bool isServerSocket(int socket);

    // 커넥션한테서 옮겨온애들
    void recvData(Connection& connection);

    void parseData(Connection& connection);
    bool parseChunk(Connection& connection);
    bool parseRequest(Connection& connection);
    std::string getChunk(std::string& recvedData);
    std::string getRequest(Connection& connection);

    void updateLastActivity(Connection& connection);

    ssize_t sendToSocket(Connection* connection);

public:
    Server(const Config& config);
    ~Server();

    void run();
};

bool isLastChunk(std::string& chunk);

#endif
