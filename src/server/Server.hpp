#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <vector>
#include "Config.hpp"
#include "Connection.hpp"
#include "EventManager.hpp"
#include "Logger.hpp"

class Server
{
private:
    const Config& config;
    EventManager eventManager;
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
    ssize_t executeByRecv(Connection& connection);

    void handleNormalRequest(Connection& connection);
    void updateLastActivity(Connection& connection);

    void handleChunkedRequest();
    std::string getCompleteChunk(Connection& connection);
    std::string getCompleteRequest(Connection& connection);

    ssize_t sendToSocket(Connection* connection);

public:
    Server(const Config& config);
    ~Server();

    void run();
};

#endif
