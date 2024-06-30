#include "Server.hpp"
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "FileManager.hpp"
#include "ChunkedRequestReader.hpp"
#include "EventManager.hpp"
#include "HttpException.hpp"
#include "Logger.hpp"
#include "SysException.hpp"
#include "RequestHandler.hpp"
#include "ResponseMessage.hpp"

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

    FileManager::setNonBlocking(serverSocket);

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

        checkKeepAlive();
    }
}

void Server::handleEvents(struct kevent& event)
{
    if (event.flags & EV_ERROR)
        return ;

    if (isServerSocket(event.ident))
    {
        acceptClient(event.ident);
    }
    else if (isConnection(event.ident))
    {
        if (event.filter == EVFILT_READ)
        {
            if (isCgiConnection(*connectionsMap[event.ident]))
            {
                handlePipeReadEvent(event);
            }
            else
            {
                handleClientReadEvent(event);
            }
        }
        else if (event.filter == EVFILT_WRITE)
        {
            if (isCgiConnection(*connectionsMap[event.ident]))
            {
                handlePipeWriteEvent(event);
            }
            else
            {
                handleClientWriteEvent(event);
            }
        }

        deleteGarbageEvent(event);
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

    FileManager::setNonBlocking(connectionSocket);

    Connection* connection = new Connection(connectionSocket);
    connectionsMap[connectionSocket] = connection;

    Logger::getInstance().logAccept(connectionSocket, clientAddr);
}

void Server::handlePipeReadEvent(struct kevent& event)
{
    Connection& cgiConnection = *(connectionsMap[event.ident]);

    if (event.flags & EV_EOF)
    {
        waitpid(-1, NULL, WNOHANG);
        ResponseMessage* response = new ResponseMessage(); // 저장된 데이터 들고 오기
        // 유효성 체크
        try
        {
            // 필수적인 헤더가 있는 지 확인
            // Status Line이나 Content-Length정도는 만들어야 할듯??
            response->parseResponseHeader(cgiConnection.recvedData);
        }
        catch(const HttpException& e)
        {
            // NOTE: 이거 찾아야됨 그럼 req에서 host를 들고 있어야함 그래야 찾을 수 있어
            response->setByStatusCode(e.getStatusCode(), config.getDefaultServerConfig());
        }
        Connection& parentConnection = *connectionsMap[cgiConnection.parentSocket]; // 부모 커넥션 가져오기
        parentConnection.responses.push(response); // 부모 커넥션의 responses에다 pipe에서 읽어 온 데이터 넣음
        EventManager::getInstance().addWriteEvent(cgiConnection.parentSocket);
        cgiConnection.recvedData.clear(); // 데이터 버퍼 클리어.

        closeConnection(event.ident);
        return;
    }

    recvData(cgiConnection);
}

void Server::handlePipeWriteEvent(struct kevent& event)
{
    int pipe = event.ident;

    Connection& cgiConnection = *connectionsMap[pipe];

    ssize_t bytesSend = 0;
    std::string& data = cgiConnection.recvedData;
    if ((bytesSend = write(pipe, data.c_str(), data.length())) < 0)
    {
        Logger::getInstance().logWarning("Failed Write to pipe");
        closeConnection(pipe);
        return ;
    }

    data.erase(0, bytesSend);
    updateLastActivity(cgiConnection);

    if (data.empty())
    {
        int readSocket = connectionsMap[cgiConnection.parentSocket]->childSocket[READ_END];
        EventManager::getInstance().addReadEvent(readSocket);
        closeConnection(pipe);
    }
}

// 소켓에 read event 발생시 소켓에서 데이터 읽음
void Server::handleClientReadEvent(struct kevent& event)
{
    int socket = event.ident;
/*
NOTE: nginx에
(echo -en "GET / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\n\r\n" && sleep 0.1) | nc localhost 9090
를 실행하면 Connection 헤더와 상관없이 EOF로 인해 Connection이 종료됨

근데 여기를 주석 처리하면 Connection이 종료되지 않음
Connection: Close 로직을 추가하고 if (event.flags & EV_EOF)&& !isKeepAlive) 로 수정해야할 듯
그리고 위의 명령어는 nginx가 예상대로 동작하지 않는 걸로 간주하고 테스트시 Conncection: Close로 테스트

또, 이거 주석 처리하면 if ((bytesRead = recv(connection.socket, buffer, sizeof(buffer) - 1, 0)) < 0)이 줄에서 이중 free 에러가 발생함
*/
    if (event.flags & EV_EOF)
    {
        return ;
    }

    Connection& connection = *connectionsMap[socket];

    recvData(connection);

    try
    {
        while (parseData(connection))
        {
            if (!connection.request)
                return ;

            size_t responseCnt = connection.responses.size(); // 1
            Logger::getInstance().logHttpMessage(*connection.request);
            RequestHandler requestHandler(connectionsMap, config, socket);
            ResponseMessage* res = requestHandler.handleRequest();
            delete connection.request;
            if (res == NULL)
                continue;

            connection.responses.push(res);
            if (connection.responses.size() > responseCnt)
                EventManager::getInstance().addWriteEvent(socket);
        }
    }
    catch(const HttpException& e)
    {
        Logger::getInstance().logHttpMessage(*connection.reqBuffer);
        ResponseMessage* res = new ResponseMessage();
        // Default error page를 위해 server config 뽑고 넣기
        const ServerConfig& serverConfig = config.getServerConfigByHost(connection.reqBuffer->getRequestHeaderFields().getField("Host"));
        res->setByStatusCode(e.getStatusCode(), serverConfig);
        connection.responses.push(res);
        EventManager::getInstance().addWriteEvent(socket);
        return ;
    }
}

// connection의 소켓으로부터 데이터를 읽어 옴
void Server::recvData(Connection& connection)
{
    char buffer[4096];
    ssize_t bytesRead;
    int socket = connection.socket;

    if ((bytesRead = read(socket, buffer, sizeof(buffer) - 1)) < 0)
    {
        Logger::getInstance().logWarning("Recv error");
        closeConnection(socket);
        return;
    }

    buffer[bytesRead] = '\0';
    connection.recvedData += std::string(buffer, buffer + bytesRead);

    updateLastActivity(connection);
}

bool Server::parseData(Connection& connection)
{
    if (connection.isChunked)
    {
        return addChunk(connection);
    }
    else if (connection.isBodyReading)
    {
        return addContent(connection);
    }
    else
    {
        RequestMessage *req = getHeader(connection);
        if (req == NULL)
            return false;

        if (!connection.isBodyReading && !connection.isChunked)
        {
            connection.reqBuffer = NULL;
            connection.request = req;
        }
        return true;
    }
}

RequestMessage* Server::getHeader(Connection& connection)
{
    size_t headerEnd = connection.recvedData.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return NULL; // \r\n\r\n 없으면 리턴

    size_t headerLen = headerEnd + 4;
    RequestMessage* req = new RequestMessage();
    connection.reqBuffer = req;
    req->parseRequestHeader(connection.recvedData.substr(0, headerLen)); // 헤더 파싱
    connection.recvedData.erase(0, headerLen);

    if (req->getRequestHeaderFields().getField("Transfer-Encoding") == "chunked")
        connection.isChunked = true;
    else if (req->getRequestHeaderFields().hasField("Content-Length"))
    {
        // 요청 헤더를 보고 serverConfig 설정
        const ServerConfig& serverConfig = config.getServerConfigByHost(req->getRequestHeaderFields().getField("Host"));
        size_t contentLength = req->getContentLength();

        if (contentLength > serverConfig.getClientMaxBodySize())
        {
            throw HttpException(CONTENT_TOO_LARGE);
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

    if (contentLength <= connection.recvedData.size())
    {
        req->addMessageBody(connection.recvedData.substr(0, contentLength));
        connection.reqBuffer = NULL;
        connection.request = req;
        connection.recvedData.erase(0, contentLength);
        connection.isBodyReading = false;

        return true;
    }
    // buffer에 아직 body가 덜 온 경우는 다음으로 기약
    return false;
}

// throwable CONTENT_TOO_LARGE
bool Server::addChunk(Connection& connection)
{
    RequestMessage* req = connection.reqBuffer;
    std::string chunk;
    bool hasData = false;

    while ((chunk = getChunk(connection)).length() > 0)
    {
        hasData = true;
        req->addMessageBody(chunk);

        if (isLastChunk(chunk))
        {
            connection.isChunked = false;  // 마지막 청크면 청크 상태 해제
            connection.reqBuffer = NULL;
            connection.request = req; // 완성된 청크를 completeRequests에 저장
            break ;
        }
    }

    return hasData;
}

// throwable CONTENT_TOO_LARGE
std::string Server::getChunk(Connection& connection)
{
    size_t pos = 0;
    size_t chunkSizeEndPos = connection.recvedData.find("\r\n", pos);
    if (chunkSizeEndPos == std::string::npos)
        return "";  // 청크 헤더가 아직 도착하지 않음

    size_t chunkSize = std::strtoul(connection.recvedData.substr(pos, chunkSizeEndPos - pos).c_str(), NULL, 16);
    pos = chunkSizeEndPos + 2;  // 청크 크기 끝을 지나서 데이터 시작

    size_t chunkEndPos = pos + chunkSize + 2;
    if (chunkEndPos > connection.recvedData.size())
        return ""; // 청크 데이터가 아직 도착하지 않음

    connection.recvedData.erase(0, chunkSizeEndPos + 2); // 청크 헤더 제거

    RequestMessage* req = connection.reqBuffer;
    if (req->getMessageBody().size() + chunkSize > connection.serverConfig.getClientMaxBodySize())
    {
        throw HttpException(CONTENT_TOO_LARGE);
    }

    // 청크 형식 불만족
    if (connection.recvedData[chunkSize] != '\r' || connection.recvedData[chunkSize + 1] != '\n')
    {
        throw HttpException(BAD_REQUEST);
    }

    std::string chunkData = connection.recvedData.substr(0, chunkSize);
    connection.recvedData.erase(0, chunkSize + 2);
    return chunkData; // 청크 데이터만 반환
}

// 이 청크가 마지막 청크인지 확인하는 로직
bool isLastChunk(std::string& chunk)
{
    if (chunk == "0\r\n\r\n")
        return true;
    return false;
}

// 보낸 데이터가 0일 때 write event 삭제
void Server::handleClientWriteEvent(struct kevent& event)
{
    int socket = event.ident;

    Connection& connection = *connectionsMap[socket];

    if (connection.responses.empty())
        return ;

    ResponseMessage* res = connection.responses.front();
    Logger::getInstance().logHttpMessage(*res);
    std::string data = res->toString();
    ssize_t bytesSend = send(connection.socket, data.c_str(), data.length(), 0);
    if (bytesSend < 0)
    {
        Logger::getInstance().logWarning("Send error");
        closeConnection(socket);
        return ;
    }

    if (res->getResponseHeaderFields().getField("Connection") == "close")
    {
        closeConnection(connection.socket);
        return ;
    }

    delete connection.responses.front();
    connection.responses.pop();

    updateLastActivity(connection);

    // if (bytesSend == 0)
        // EventManager::getInstance().deleteWriteEvent(socket);
}

// Connection들의 keepAlive 관리
void Server::checkKeepAlive()
{
    const time_t now = time(NULL);
    std::queue<int> closeSockets;

    for (std::map<int, Connection*>::const_iterator it = connectionsMap.begin(); it != connectionsMap.end(); ++it)
    {
        if ((it->second)->parentSocket != -1) // pipe 커넥션이면 패스
            continue ;

        if (difftime(now, (it->second)->last_activity) > config.getKeepAliveTime())
		{
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

void Server::updateLastActivity(Connection& connection)
{
    connection.last_activity = time(NULL);
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

bool Server::isConnection(int key)
{
    std::map<int, Connection*>::iterator it = connectionsMap.find(key);
    return (it != connectionsMap.end());
}

void Server::deleteGarbageEvent(struct kevent& event)
{
    if (isConnection(event.ident))
        return ;

    if (event.filter == EVFILT_READ)
    {
        EventManager::getInstance().deleteReadEvent(event.ident);
    }
    else if (event.filter == EVFILT_WRITE)
    {
        EventManager::getInstance().deleteWriteEvent(event.ident);
    }
}
