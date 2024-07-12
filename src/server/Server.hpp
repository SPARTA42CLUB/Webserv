#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <vector>
#include "Config.hpp"
#include "Connection.hpp"

#define BUFFER_SIZE 4096

class Server
{
private:
    const Config& config;
    std::vector<int> serverSockets;
    std::map<int, ServerConfig> socketToConfig;
    std::map<int, Connection*> connectionsMap;

    int setServerSocket(ServerConfig serverConfig);

    void manageTimeout();

    void handleEvents(struct kevent& event);
    void handleClientsEvent(struct kevent& event);

    void acceptClient(int serverSocket);
    void handleClientReadEvent(struct kevent& event);
    void handleClientWriteEvent(struct kevent& event);
    void handlePipeReadEvent(struct kevent& event);
    void handlePipeWriteEvent(struct kevent& event);
    void handleProxyReadEvent(struct kevent& event);
    void handleProxyWriteEvent(struct kevent& event);

    void moveDataToParent(Connection& connection);
    void closeCgi(Connection& connection);
    bool readData(Connection& connection);

    bool buildMessage(Connection& connection);
    RequestMessage* getHeader(Connection& connection);
    bool addContent(Connection& connection);
    bool addChunk(Connection& connection);
    std::string getChunk(Connection& connection);

    void closeConnection(const int socket);
    bool isConnection(const int socket);
    bool isCgiConnection(const Connection* connection);
    bool isProxyConnection(const Connection* connection);
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
