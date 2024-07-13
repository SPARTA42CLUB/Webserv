#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <vector>
#include "Config.hpp"
#include "Connection.hpp"
#include "EventManager.hpp"

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

    void handleEvents(struct EVENT_TYPE& event);
    void handleClientsEvent(struct EVENT_TYPE& event);

    void acceptClient(int serverSocket);
    void handleClientReadEvent(struct EVENT_TYPE& event);
    void handleClientWriteEvent(struct EVENT_TYPE& event);
    void handlePipeReadEvent(struct EVENT_TYPE& event);
    void movePipeDataToParent(Connection& cgiConnection);
    void closeCgi(Connection& connection);
    void handlePipeWriteEvent(struct EVENT_TYPE& event);

    bool readData(Connection& connection);

    bool buildMessage(Connection& connection);
    RequestMessage* getHeader(Connection& connection);
    bool addContent(Connection& connection);
    bool addChunk(Connection& connection);
    std::string getChunk(Connection& connection);

    void closeConnection(const int socket);
    bool isConnection(const int socket);
    bool isServerSocket(int socket);
    void updateLastActivity(Connection& connection);

    void deleteGarbageEvent(struct EVENT_TYPE& event);

    void pushResponse(Connection& connection, int statusCode);

public:
    Server(const Config& config);
    ~Server();

    void run();
};

#endif
