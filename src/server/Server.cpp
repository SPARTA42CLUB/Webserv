#include "Server.hpp"
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include "ChunkedRequestReader.hpp"
#include "EventManager.hpp"
#include "HTTPException.hpp"
#include "Logger.hpp"
#include "SysException.hpp"

// ServerConfig 클래스 생성자
Server::Server(const Config& config)
: config(config)
, serverSockets()
, connectionsMap()
{
    setupServerSockets();
}

Server::~Server()
{
    for (size_t i = 0; i < serverSockets.size(); ++i)
    {
        close(serverSockets[i]);
    }

    for (std::map<int, Connection*>::const_iterator it = connectionsMap.begin(); it != connectionsMap.end(); ++it)
    {
        closeConnection(it->first);
    }
}

// Server 소켓 생성 및 설정
void Server::setupServerSockets()
{
    /*
     * 다중 서버 소켓을 관리하는 이유 : 하나의 서버 소켓이 하나의 port에 대한 연결 요청을 처리한다.
     * 예시로 443(https)포트와 8080(http)포트를 동시에 사용하는 경우, 서버 소켓을 2개 생성하여 각각의 포트에 대한 연결 요청을 처리한다.
     * 또한 서버는 여러개의 IP를 가질 수 있으므로, 서버 소켓을 여러개 생성하여 각각의 IP에 대한 연결 요청을 처리할 수 있다.
     * 서버가 여러개의 IP를 가지는 경우는 서버가 여러개의 네트워크 인터페이스를 가지는 경우이다.
     */
    std::vector<ServerConfig> serverConfigs = config.getServerConfigs();

    for (size_t i = 0; i < serverConfigs.size(); ++i)
    {
        int serverSocket = createServerSocket(serverConfigs[i]);

        if (listen(serverSocket, 10) == -1)
        {
            close(serverSocket);
            throw SysException(FAILED_TO_LISTEN_SOCKET);
        }

        serverSockets.push_back(serverSocket);

        EventManager::getInstance().addReadEvent(serverSocket);
    }
}

// 소켓 생성 및 config에 따른 binding
int Server::createServerSocket(ServerConfig serverConfig)
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
        throw SysException(FAILED_TO_CREATE_SOCKET);

    /* 개발 편의용 세팅. 서버 소켓이 이미 사용중이더라도 실행되게끔 설정 */
    int optval = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    /* ----------------------------------------------------------------- */

    setNonBlocking(serverSocket);

    // 서버 주소 설정
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));                           // 0으로 초기화
    serverAddr.sin_family = AF_INET;                                      // IPv4
    serverAddr.sin_port = htons(serverConfig.port);                       // 포트 설정
    inet_pton(AF_INET, serverConfig.host.c_str(), &serverAddr.sin_addr);  // IP 주소 설정 NOTE: host 검색해보기

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
    {
        close(serverSocket);
        throw SysException(FAILED_TO_BIND_SOCKET);
    }

    return serverSocket;
}

// 소켓 논블로킹 설정
void Server::setNonBlocking(int socket)
{
    if (fcntl(socket, F_SETFL, O_NONBLOCK) == -1)
    {
        close(socket);
        throw SysException(FAILED_TO_SET_NON_BLOCKING);
    }
}

// 서버 실행
void Server::run()
{
    // 이벤트 처리 루프
    while (true)
    {
        std::vector<struct kevent> events = EventManager::getInstance().getCurrentEvents();
        for (std::vector<struct kevent>::iterator it = events.begin(); it != events.end(); ++it)
        {
            struct kevent& event = *it;

            if (event.flags & EV_ERROR)
                throw SysException(KEVENT_ERROR);

            if (isServerSocket(event.ident))
            {
                acceptClient(event.ident);
            }
            else if (event.filter == EVFILT_READ)
            {
                handleClientReadEvent(event);
            }
            else if (event.filter == EVFILT_WRITE)
            {
                handleClientWriteEvent(event);
            }
        }

        checkKeepAlive();
    }
}

// Connection들의 keepAlive 관리
void Server::checkKeepAlive()
{
    const time_t now = time(NULL);
    for (std::map<int, Connection*>::const_iterator it = connectionsMap.begin(); it != connectionsMap.end(); ++it)
    {
        if (difftime(now, (it->second)->last_activity) > config.getKeepAliveTime())
            closeConnection(it->first);
    }
}

// 서버 소켓에서 읽기 이벤트가 발생했다는 것의 의미 : 새로운 클라이언트가 연결 요청을 보냈다는 것
void Server::acceptClient(int serverSocket)
{
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    int connectionSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
    if (connectionSocket == -1)
    {
        Logger::getInstance().logWarning("Failed to accept");
        return;
    }

    EventManager::getInstance().addReadEvent(connectionSocket);

    setNonBlocking(connectionSocket);

    Connection* connection = new Connection(connectionSocket);
    connectionsMap[connectionSocket] = connection;

    Logger::getInstance().logAccept(connectionSocket, clientAddr);
}

// 소켓에 read event 발생시 소켓에서 데이터 읽음
void Server::handleClientReadEvent(struct kevent& event)
{
    if (event.flags & EV_EOF)
    {
        closeConnection(event.ident);
        return;
    }
    Connection& connection = *connectionsMap[event.ident];

    const ssize_t recvBytes = executeByRecv(connection);
    if (recvBytes < 0)
    {
        Logger::getInstance().logWarning("Recv error");
        closeConnection(event.ident);
    }
}

ssize_t Server::executeByRecv(Connection& connection)
{
    char buffer[4096];
    ssize_t bytesRead;

    if ((bytesRead = recv(connection.socket, buffer, sizeof(buffer) - 1, 0)) <= 0)
        return bytesRead;

    buffer[bytesRead] = '\0';
    connection.recvData += std::string(buffer, buffer + bytesRead);

    // 청크 인코딩 여부에 따라 청크 요청 처리
    if (connection.isChunked)
    {
        std::string chunk;

        while ((chunk = getCompleteChunk(connection)).length() > 0)
        {
            // NOTE: 헤더 보고 파일 경로 수정해야 함.
            ChunkedRequestReader reader("upload/testfile.png", chunk);
            bool isChunkedEnd = reader.processRequest();

            connection.recvData.erase(0, chunk.length());

            if (isChunkedEnd)
            {
                connection.isChunked = false;  // 마지막 청크 후 청크 상태 해제
                return chunk.length();
            }
        }
    }

    // 일반 요청 처리
    handleNormalRequest(connection);

    updateLastActivity(connection);
    return bytesRead;
}

// // NOTE: 여기는 RequestHandler 손보고 고쳐야 할듯..
void Server::handleNormalRequest(Connection& connection)
{
    (void)connection;
    // std::string requestString;
    // while ((requestString = getCompleteRequest(connection)).length() > 0)
    // {
    //     connection->recvData.erase(0, requestString.length());

    //     RequestMessage reqMsg(requestString);
    //     RequestHandler requestHandler;
    //     requestHandler.handleRequest(reqMsg);
    // }

    // ResponseMessage* res = new ResponseMessage();
    // ResponseMessage& resMsg = *res;
    // RequestHandler requestHandler(resMsg); // NOTE: RequestHandler 수정돼야 함.

    // try
    // {

    //     if (reqMsg.getRequestHeaderFields().getField("Transfer-Encoding") == "chunked") {
    //         delete res;
    //         recvedData.erase(0, requestLength);
    //         isChunked = true;

    //         handleChunkedRequest();
    //         return ;
    //     }

    //     Logger::getInstance().logHTTPMessage(*res, completeRequest);
    // }
    // catch (const HTTPException& e)
    // {
    //     requestHandler.handleException(e);
    // }

    // responses.push_back(res);

    // recvedData.erase(0, requestLength);

    // // 리소스 효율을 위해 요청이 하나 완료되면 그 때 WRITE 이벤트 등록
    // if (!responses.empty()) {
    //     EventManager::getInstance().addWriteEvent(socket);
    // }
}

void Server::updateLastActivity(Connection& connection)
{
    connection.last_activity = time(NULL);
}

// 보낸 데이터가 0일 때 write event 삭제
void Server::handleClientWriteEvent(struct kevent& event)
{
    int socket = event.ident;

    const ssize_t bytesSend = sendToSocket(connectionsMap[socket]);
    if (bytesSend < 0)
    {
        Logger::getInstance().logWarning("Send error");
        closeConnection(socket);
    }

    if (bytesSend == 0)
        EventManager::getInstance().deleteWriteEvent(socket);
}

// 커넥션 종료에 필요한 작업들 처리
void Server::closeConnection(int socket)
{
    delete connectionsMap[socket];
    connectionsMap.erase(socket);
}

bool Server::isServerSocket(int socket)
{
    if (std::find(serverSockets.begin(), serverSockets.end(), socket) != serverSockets.end())
        return true;

    return false;
}

// NOTE: 그냥 2 더하면 안되고 \r\n인지 검사해야 함.
std::string Server::getCompleteChunk(Connection& connection)
{
    size_t pos = 0;
    size_t chunkSizeEnd = connection.recvData.find("\r\n", pos);
    if (chunkSizeEnd == std::string::npos)
        return "";  // 청크 크기가 아직 도착하지 않음""

    size_t chunkSize = std::strtoul(connection.recvData.substr(pos, chunkSizeEnd - pos).c_str(), NULL, 16);
    pos = chunkSizeEnd + 2;  // 청크 크기 끝을 지나서 데이터 시작

    if (pos + chunkSize + 2 > connection.recvData.size())
        return "";  // 청크 데이터가 아직 도착하지 않음
    return connection.recvData.substr(0, pos + chunkSize + 2);
}

// NOTE:그냥 4 더 하면 안 되고 \r\n\r\n인지 확인해야 함
std::string Server::getCompleteRequest(Connection& connection)
{
    size_t headerEnd = connection.recvData.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return "";

    size_t requestLength = headerEnd + 4;

    std::istringstream headerStream(connection.recvData.substr(0, headerEnd + 4));
    std::string headerLine;
    while (std::getline(headerStream, headerLine) && headerLine != "\r")
    {
        if (headerLine.find("Content-Length:") != std::string::npos)
        {
            requestLength += std::strtoul(headerLine.substr(16).c_str(), NULL, 10);
            break;
        }
    }

    if (connection.recvData.length() >= requestLength)
        return connection.recvData.substr(0, requestLength);
    return "";
}

// NOTE: 응답 생성할 때마다 keepalive 설정하게 변경해야 함
ssize_t Server::sendToSocket(Connection* connection)
{
    std::queue<ResponseMessage*>& responses = connection->responses;
    if (responses.empty())
        return 0;

    std::string data = responses.front()->toString();
    ssize_t bytesSend = send(connection->socket, data.c_str(), data.length(), 0);

    delete responses.front();
    responses.pop();

    updateLastActivity(*connection);

    return bytesSend;
}
