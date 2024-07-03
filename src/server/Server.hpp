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
    std::map<int, ServerConfig> socketToConfig;
    std::map<int, Connection*> connectionsMap;

    int setServerSocket(ServerConfig serverConfig);

    void checkKeepAlive();

    void handleEvents(struct kevent& event);

    void acceptClient(int serverSocket);
    void handleClientReadEvent(struct kevent& event);
    void handleClientWriteEvent(struct kevent& event);
    void handlePipeReadEvent(struct kevent& event);
    void movePipeDataToParent(Connection& cgiConnection);
    void closeCgi(Connection& connection);
    void handlePipeWriteEvent(struct kevent& event);

    bool recvData(Connection& connection);

    bool parseData(Connection& connection);
    RequestMessage* getHeader(Connection& connection);
    bool addContent(Connection& connection);
    bool addChunk(Connection& connection);
    std::string getChunk(Connection& connection);

    void closeConnection(int socket);
    bool isConnection(int key);
    bool isServerSocket(int socket);
    void updateLastActivity(Connection& connection);

    void deleteGarbageEvent(struct kevent& event);

    void pushResponse(Connection& connection, int statusCode);

public:
    Server(const Config& config);
    ~Server();

    void run();
};

#endif
