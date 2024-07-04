#include "Server.hpp"
#include <arpa/inet.h>
#include <signal.h>
#include "EventManager.hpp"
#include "FileManager.hpp"
#include "HttpException.hpp"
#include "Logger.hpp"
#include "RequestHandler.hpp"
#include "ResponseMessage.hpp"
#include "SysException.hpp"
#include <iostream>

// ServerConfig 클래스 생성자
Server::Server(const Config& config)
: config(config)
, serverSockets()
, socketToConfig()
, connectionsMap()
{
    std::vector<ServerConfig> serverConfigs = config.getServerConfigs();

    for (size_t i = 0; i < serverConfigs.size(); ++i)
    {
        int serverSocket = setServerSocket(serverConfigs[i]);

        // https://stackoverflow.com/questions/18073483/what-do-somaxconn-mean-in-c-socket-programming
        if (listen(serverSocket, SOMAXCONN) == -1)
        {
            close(serverSocket);
            throw SysException(FAILED_TO_LISTEN_SOCKET);
        }

        serverSockets.push_back(serverSocket);
        socketToConfig[serverSocket] = serverConfigs[i];

        EventManager::getInstance().addReadEvent(serverSocket);
    }
}

Server::~Server()
{
    std::vector<int> closeSockets;
    for (size_t i = 0; i < serverSockets.size(); ++i)
    {
        close(serverSockets[i]);
    }

    for (std::map<int, Connection*>::const_iterator it = connectionsMap.begin(); it != connectionsMap.end(); ++it)
    {
        delete it->second;
    }
}

// 소켓 생성 및 config에 따른 binding
int Server::setServerSocket(ServerConfig serverConfig)
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
        throw SysException(FAILED_TO_CREATE_SOCKET);

    /* NOTE: 개발 편의용 세팅. 서버 소켓이 이미 사용중이더라도 실행되게끔 설정 */
    int optval = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    /* ----------------------------------------------------------------- */

    fileManager::setNonBlocking(serverSocket);

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

            handleEvents(event);
        }

        manageTimeout();
    }
}

void Server::handleEvents(struct kevent& event)
{
    if (event.flags & EV_ERROR)
        return;

    if (isServerSocket(event.ident))
    {
        acceptClient(event.ident);
    }
    else if (isConnection(event.ident))
    {
        handleClientsEvent(event);

        deleteGarbageEvent(event);
    }
}

void Server::handleClientsEvent(struct kevent& event)
{
    if (event.filter == EVFILT_READ)
    {
        if (isCgiConnection(connectionsMap[event.ident]))
            handlePipeReadEvent(event);
        else
            handleClientReadEvent(event);
        return ;
    }

    if (event.filter == EVFILT_WRITE)
    {
        if (isCgiConnection(connectionsMap[event.ident]))
            handlePipeWriteEvent(event);
        else
            handleClientWriteEvent(event);
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

    fileManager::setNonBlocking(connectionSocket);

    Connection* connection = new Connection(connectionSocket, socketToConfig[serverSocket]);
    connectionsMap[connectionSocket] = connection;

    Logger::getInstance().logAccept(connectionSocket, clientAddr);
}

void Server::handlePipeReadEvent(struct kevent& event)
{
    Connection& cgiConnection = *(connectionsMap[event.ident]);

    if (event.flags & EV_EOF)
    {
        movePipeDataToParent(cgiConnection);
        closeCgi(*connectionsMap[cgiConnection.parentSocket]);
        return;
    }

    readData(cgiConnection);
}

void Server::closeCgi(Connection& connection)
{
    kill(connection.cgiPid, SIGKILL);
    waitpid(connection.cgiPid, NULL, 0);

    closeConnection(connection.childSocket[READ_END]);
    closeConnection(connection.childSocket[WRITE_END]);

    connection.cgiPid = -1;
    connection.childSocket[READ_END] = -1;
    connection.childSocket[WRITE_END] = -1;
}

void Server::movePipeDataToParent(Connection& cgiConnection)
{
    Connection& parentConnection = *connectionsMap[cgiConnection.parentSocket];  // 부모 커넥션 가져오기

    ResponseMessage* response = new ResponseMessage();

    try
    {
        response->parseResponseMessage(cgiConnection.buffer);
    }
    catch (const HttpException& e)
    {
        response->setByStatusCode(e.getStatusCode(), cgiConnection.serverConfig);
    }

    parentConnection.responses.push(response);  // 부모 커넥션의 responses에다 pipe에서 읽어 온 데이터 넣음
    EventManager::getInstance().addWriteEvent(cgiConnection.parentSocket);
    cgiConnection.buffer.clear();  // 데이터 버퍼 클리어.
}

void Server::handlePipeWriteEvent(struct kevent& event)
{
    int pipe = event.ident;

    Connection& cgiConnection = *connectionsMap[pipe];

    ssize_t writeSize;
    std::string& data = cgiConnection.buffer;
    if ((writeSize = write(pipe, data.c_str(), data.size())) < 0)
    {
        // write 에러 시 커넥션 종료
        Logger::getInstance().logWarning("Failed Write to pipe");
        closeConnection(pipe);
        return;
    }

    data.erase(0, writeSize);
    updateLastActivity(cgiConnection);

    // GET요청이거나 바디를 다 전송했을 시
    if (writeSize == 0 || data.empty())
    {
        int readSocket = connectionsMap[cgiConnection.parentSocket]->childSocket[READ_END];
        EventManager::getInstance().addReadEvent(readSocket);
        closeConnection(pipe);
    }
}

// 소켓에 read event 발생시 소켓에서 데이터 읽음
void Server::handleClientReadEvent(struct kevent& event)
{
    const int socket = event.ident;

    if (event.flags & EV_EOF)
        return;

    Connection& connection = *connectionsMap[socket];
    if (connection.isKeepAlive == false)
        return;

    if (readData(connection) == false)
        return;

    updateLastActivity(connection);

    while (buildMessage(connection))
    {
        if (!connection.request)
            continue;

        // Request에 따른 처리
        if (connection.request->getRequestHeaderFields().getField("Connection") == "close")
            connection.isKeepAlive = false;
        std::string method = connection.request->getRequestLine().getMethod();
        Logger::getInstance().logHttpMessage(connection.request);

        // 요청 처리
        RequestHandler requestHandler(connectionsMap, socket);
        ResponseMessage* res = requestHandler.handleRequest();

        // Request에 따른 처리가 완료 되었으니 자원 회수
        delete connection.request;
        connection.request = NULL;

        // Request가 Cgi 요청이었던 경우 RequestHandler가 NULL을 반환함
        if (res == NULL)
            continue;

        if (method == "HEAD")
            res->clearMessageBody();

        connection.responses.push(res);
        EventManager::getInstance().addWriteEvent(socket);
    }
}

// connection의 소켓으로부터 데이터를 읽어 옴
bool Server::readData(Connection& connection)
{
    char buf[BUFFER_SIZE];
    ssize_t readSize;
    int socket = connection.socket;

    if ((readSize = read(socket, buf, BUFFER_SIZE - 1)) < 0)
    {
        Logger::getInstance().logWarning("Recv error");
        closeConnection(socket);
        return false;
    }

    // 소켓으로부터 EOF를 읽음
    if (readSize == 0)
    {
        return false;
    }

    buf[readSize] = '\0';
    connection.buffer += std::string(buf, buf + readSize);

    return true;
}

bool Server::buildMessage(Connection& connection)
{
    if (connection.isInChunkStream)
        return addChunk(connection);

    if (connection.isBodyReading)
        return addContent(connection);

    RequestMessage* req = getHeader(connection);
    if (req == NULL)
        return false;

    // 안에서 에러가 발생했거나 온전한 헤더가 만들어졌거나
    if (req->getStatusCode() != OK || (!connection.isBodyReading && !connection.isInChunkStream))
    {
        connection.reqBuffer = NULL;
        connection.request = req;
    }

    return true;
}

RequestMessage* Server::getHeader(Connection& connection)
{
    size_t headerEnd = connection.buffer.find(HEADER_END);
    if (headerEnd == std::string::npos)
        return NULL;

    size_t headerLen = headerEnd + 4;
    RequestMessage* req = new RequestMessage();
    connection.reqBuffer = req;
    req->parseRequestHeader(connection.buffer.substr(0, headerLen));  // 헤더 파싱
    connection.buffer.erase(0, headerLen);
    if (req->getStatusCode() != OK)
        return req;

    if (req->getRequestHeaderFields().getField("Transfer-Encoding") == "chunked")
        connection.isInChunkStream = true;
    else if (req->getRequestHeaderFields().hasField("Content-Length"))
    {
        size_t contentLength = req->getContentLength();
        if (contentLength > connection.serverConfig.client_max_body_size)
        {
            req->setStatusCode(CONTENT_TOO_LARGE);
            return req;
        }

        if (contentLength > 0)
            connection.isBodyReading = true;
    }

    return req;
}
bool Server::addContent(Connection& connection)
{
    RequestMessage* req = connection.reqBuffer;
    size_t contentLength = req->getContentLength();

    if (contentLength <= connection.buffer.size())
    {
        req->addMessageBody(connection.buffer.substr(0, contentLength));
        connection.reqBuffer = NULL;
        connection.request = req;
        connection.buffer.erase(0, contentLength);
        connection.isBodyReading = false;

        return true;
    }
    // buffer에 아직 body가 덜 온 경우는 다음으로 기약
    return false;
}

bool Server::addChunk(Connection& connection)
{
    RequestMessage* req = connection.reqBuffer;
    std::string chunk;
    bool hasData = false;

    while ((chunk = getChunk(connection)).length() > 0)
    {
        hasData = true;
        req->addMessageBody(chunk);
    }

    // 안에서 error 발생했을 시
    if (connection.reqBuffer->getStatusCode() != OK)
    {
        connection.isInChunkStream = false;
        connection.reqBuffer = NULL;
        connection.request = req;
        return true;
    }

    // getChunk 내부에서 마지막 청크를 받았으면
    if (connection.isInChunkStream == false)
        return true ;

    return hasData;
}

std::string Server::getChunk(Connection& connection)
{
    size_t pos = 0;
    size_t chunkSizeEndPos = connection.buffer.find(CRLF, pos);
    if (chunkSizeEndPos == std::string::npos)
        return "";  // 청크 헤더가 아직 도착하지 않음

    size_t chunkSize = std::strtoul(connection.buffer.substr(pos, chunkSizeEndPos - pos).c_str(), NULL, 16);
    pos = chunkSizeEndPos + 2;  // 청크 크기 끝을 지나서 데이터 시작

    size_t chunkEndPos = pos + chunkSize + 2;
    if (chunkEndPos > connection.buffer.size())
        return ""; // 청크 데이터가 아직 도착하지 않음

    connection.buffer.erase(0, chunkSizeEndPos + 2); // 청크 헤더 제거

    RequestMessage* req = connection.reqBuffer;
    if (req->getMessageBody().size() + chunkSize > connection.serverConfig.client_max_body_size)
    {
        req->setStatusCode(CONTENT_TOO_LARGE);
        return "";
    }

    // 청크 형식 불만족
    if (connection.buffer[chunkSize] != '\r' || connection.buffer[chunkSize + 1] != '\n')
    {
        req->setStatusCode(BAD_REQUEST);
        return "";
    }

    std::string chunkData = connection.buffer.substr(0, chunkSize);
    connection.buffer.erase(0, chunkSize + 2);

    // 마지막 청크면
    if (chunkSize == 0)
    {
        connection.isInChunkStream = false;  // 마지막 청크면 청크 상태 해제
        connection.reqBuffer = NULL;
        connection.request = req; // 완성된 청크를 completeRequests에 저장
    }
    return chunkData; // 청크 데이터만 반환
}

// 보낸 데이터가 0일 때 write event 삭제
void Server::handleClientWriteEvent(struct kevent& event)
{
    int socket = event.ident;

    Connection& connection = *connectionsMap[socket];

    if (connection.responses.empty())
        return;

    ResponseMessage* res = connection.responses.front();
    res->setConnection(connection); // Client의 Connection 필드 값, 남은 responses 유무, Response의 에러 코드를 보고 res에 Connection 필드 생성
    Logger::getInstance().logHttpMessage(res);

    std::string data = res->toString();
    ssize_t sendSize = send(connection.socket, data.c_str(), data.length(), 0);
    if (sendSize < 0)
    {
        Logger::getInstance().logWarning("Send error");
        closeConnection(socket);
        return;
    }

    // setConnection에서 생성한 Connection 필드 값에 따라 실제 Connecion closing
    if (res->isConnectionClose())
    {
        closeConnection(connection.socket);
        return;
    }

    delete connection.responses.front();
    connection.responses.pop();

    updateLastActivity(connection);

    if (sendSize == 0)
        EventManager::getInstance().deleteWriteEvent(socket);
}

// Timeout을 검사해 자원 관리
void Server::manageTimeout()
{
    const time_t now = time(NULL);
    std::queue<int> closeSockets;

    for (std::map<int, Connection*>::const_iterator it = connectionsMap.begin(); it != connectionsMap.end(); ++it)
    {
        Connection& connection = *(it->second);

        if (connection.parentSocket != -1)  // cgi 커넥션이면 pass
        {
            continue ;
        }

        // 딸린 CGI 커넥션이 있는 경우 CGI부터 먼저 처리
        if (connection.cgiPid != -1)
        {
            // CGI의 응답이 너무 오래 걸리는 경우 CGI를 강제 종료시키고 BAD_GATEWAY를 띄움

            Connection& cgiConnection = *connectionsMap[connection.childSocket[READ_END]];
            if (difftime(now, cgiConnection.last_activity) > config.getCgiTimeout())
            {
                pushResponse(connection, BAD_GATEWAY);
                cgiConnection.buffer.clear();  // 데이터 버퍼 클리어.
                closeCgi(connection);
            }
        }

        if (difftime(now, connection.last_activity) > config.getKeepAliveTime())
        {
            // 아직 처리할 응답이 남아있는 경우 살려줌
            if (!connection.responses.empty())
                continue;

            delete connectionsMap[it->first];
            closeSockets.push(it->first);
        }
    }

    while (!closeSockets.empty())
    {
        connectionsMap.erase(closeSockets.front());
        closeSockets.pop();
    }
}

void Server::pushResponse(Connection& connection, int statusCode)
{
    ResponseMessage* response = new ResponseMessage();
    response->setByStatusCode(statusCode, connection.serverConfig);
    connection.responses.push(response);
    EventManager::getInstance().addWriteEvent(connection.socket);
}

void Server::updateLastActivity(Connection& connection)
{
    connection.last_activity = time(NULL);
}

// 커넥션 종료에 필요한 작업들 처리
void Server::closeConnection(int socket)
{
    if (isConnection(socket) == false)
        return ;

    Connection& connection = *connectionsMap[socket];
    if (connection.cgiPid != -1)
    {
        closeCgi(connection);
    }

    delete connectionsMap[socket];
    connectionsMap.erase(socket);
}

bool Server::isServerSocket(int socket)
{
    if (std::find(serverSockets.begin(), serverSockets.end(), socket) != serverSockets.end())
        return true;

    return false;
}

bool Server::isConnection(int key)
{
    std::map<int, Connection*>::const_iterator it = connectionsMap.find(key);
    return (it != connectionsMap.end());
}

void Server::deleteGarbageEvent(struct kevent& event)
{
    if (isConnection(event.ident))
        return;

    if (event.filter == EVFILT_READ)
    {
        EventManager::getInstance().deleteReadEvent(event.ident);
    }
    else if (event.filter == EVFILT_WRITE)
    {
        EventManager::getInstance().deleteWriteEvent(event.ident);
    }
}
