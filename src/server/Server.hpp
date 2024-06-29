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

    void checkKeepAlive();

    void handleEvents(struct kevent& event);

    void acceptClient(int serverSocket);
    void handleClientReadEvent(struct kevent& event);
    void handleClientWriteEvent(struct kevent& event);
    void handlePipeReadEvent(struct kevent& event);
    void handlePipeWriteEvent(struct kevent& event);

    void recvData(Connection& connection);
    ssize_t sendToSocket(Connection& connection);

    void parseData(Connection& connection);
    bool parseChunk(Connection& connection);
    bool parseRequest(Connection& connection);
    std::string getChunk(Connection& connection);
    std::string getRequest(Connection& connection);

    void closeConnection(int socket);
    bool isConnection(int key);
    bool isServerSocket(int socket);
    void updateLastActivity(Connection& connection);

    void deleteGarbageEvent(struct kevent& event);

public:
    Server(const Config& config);
    ~Server();

    void run();
};

bool isLastChunk(std::string& chunk);

#endif
