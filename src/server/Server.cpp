#include "Server.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "Exception.hpp"
#include "HTTPException.hpp"
#include "RequestHandler.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "ChunkedRequestReader.hpp"

// ServerConfig 클래스 생성자
Server::Server(const Config& config)
: config(config)
, eventManager()
{
    // Server 소켓 생성 및 설정
    setupServerSockets();
}

Server::~Server()
{
    for (size_t i = 0; i < serverSockets.size(); ++i)
    {
        close(serverSockets[i]);
    }

    for (size_t i = 0; i < connections.size(); ++i)
        close(connections[i]);
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

        // 소켓 리스닝
        if (listen(serverSocket, 10) == -1)
        {
            close(serverSocket);
            throw Exception(FAILED_TO_LISTEN_SOCKET);
        }

        // 서버 소켓 벡터에 추가
        serverSockets.push_back(serverSocket);

        eventManager.addReadEvent(serverSocket);
    }
}

int Server::createServerSocket(ServerConfig serverConfig) {
    // 소켓 생성
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
        throw std::runtime_error("Failed to create socket");

    /* 개발 편의용 세팅. 서버 소켓이 이미 사용중이더라도 실행되게끔 설정 */
    int optval = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    /* ----------------------------------------------------------------- */

    // 소켓 옵션 설정
    setNonBlocking(serverSocket);

    // 서버 주소 설정
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));                               // 0으로 초기화
    serverAddr.sin_family = AF_INET;                                          // IPv4
    serverAddr.sin_port = htons(serverConfig.port);                       // 포트 설정
    inet_pton(AF_INET, serverConfig.host.c_str(), &serverAddr.sin_addr);  // IP 주소 설정

    // 소켓 바인딩
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
    {
        close(serverSocket);
        throw Exception(FAILED_TO_BIND_SOCKET);
    }

    return serverSocket;
}

// 소켓 논블로킹 설정
void Server::setNonBlocking(int socket)
{
    // 논블로킹 설정
    if (fcntl(socket, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("Failed to set non-blocking");
}

// 서버 실행
void Server::run()
{
    // 이벤트 처리 루프
    while (true)
    {
        std::vector<struct kevent> events = eventManager.getCurrentEvents();
        for (std::vector<struct kevent>::iterator it = events.begin(); it != events.end(); ++it)
        {
            struct kevent& event = *it;

            if (event.flags & EV_ERROR)
            {
                std::cerr << "Event error: " << strerror(event.data) << std::endl;
                close(event.ident);
                continue;
            }

            if (std::find(serverSockets.begin(), serverSockets.end(), event.ident) != serverSockets.end())
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

        checkTimeout();
    }
}

void Server::checkTimeout()
{
    time_t now = time(NULL);

    for (size_t i = 0; i < connections.size(); ++i)
    {
        int connection = connections[i];
        int timeout = socketToConfigMap[connection].keepalive_timeout;

        if (difftime(now, last_activity_map[connection]) > timeout)
        {
            std::cout << "Timeout. Connection closed: " << connection << std::endl;
            closeConnection(connection);
        }
    }
}

// 서버 소켓으로 읽기 이벤트가 발생했다는 것의 의미 : 새로운 클라이언트가 연결 요청을 보냈다는 것
// 서버 소켓인 경우 'accept'를 이용하여 클라이언트의 연결 수락
void Server::acceptClient(int serverSocket)
{
    //
    update_last_activity(connection);

    try
    {
        eventManager.addReadEvent(connection);
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
        close(connection);
        return;
    }

    connections.push_back(connection);
    socketToConfigMap[connection] = socketToConfigMap[serverSocket];
    isChunkedMap[connection] = false;
}

// 클라이언트 요청 처리
void Server::handleClientReadEvent(struct kevent& event) {
    // if (event.flags & EV_EOF)
    //     return;

    int socket = event.ident;

    update_last_activity(socket);

    // 클라이언트 요청 수신
    char buffer[4096];
    ssize_t bytesRead;
    std::string& requestData = recvDataMap[socket];

    if ((bytesRead = recv(socket, buffer, sizeof(buffer) - 1, 0)) < 0) {
        std::cerr << "recv error" << std::endl;
        closeConnection(socket);
        return;
    }

    if (bytesRead == 0) {
        return ;
    }

    buffer[bytesRead] = '\0';
	requestData += std::string(buffer, buffer + bytesRead);

    // 청크 인코딩 여부에 따라 청크 요청 처리
    if (isChunkedMap[socket]) {
        handleChunkedRequest(socket, requestData);
    }

    size_t requestLength = 0;
    // 일반 요청 처리
    while (isCompleteRequest(requestData, requestLength)) {
        handleNormalRequest(socket, requestData, requestLength);
    }
}

bool Server::isCompleteRequest(const std::string& data, size_t& requestLength) {
    size_t headerEnd = data.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return false;

    requestLength = headerEnd + 4;

    std::istringstream headerStream(data.substr(0, headerEnd + 4));
    std::string headerLine;
    while (std::getline(headerStream, headerLine) && headerLine != "\r") {
        if (headerLine.find("Content-Length:") != std::string::npos) {
            requestLength += std::strtoul(headerLine.substr(16).c_str(), NULL, 10);
            return data.size() >= requestLength;
        }
    }

    return data.size() >= requestLength;
}

void Server::handleNormalRequest(int socket, std::string& requestData, size_t requestLength) {
    std::string completeRequest = requestData.substr(0, requestLength);

    ResponseMessage* res = new ResponseMessage();
    ResponseMessage& resMsg = *res;
    RequestHandler requestHandler(resMsg, socketToConfigMap[socket]);

    try
    {
        RequestMessage reqMsg(completeRequest);
        requestHandler.handleRequest(reqMsg);

        if (reqMsg.getRequestHeaderFields().getField("Transfer-Encoding") == "chunked") {
            delete res;
            requestData.erase(0, requestLength);
            isChunkedMap[socket] = true;

            handleChunkedRequest(socket, requestData);
            return ;
        }
    }
    catch (const HTTPException& e)
    {
        requestHandler.handleException(e);
    }

    responsesMap[socket].push_back(res);
    logHTTPMessage(socket, *res, completeRequest);

    requestData.erase(0, requestLength);

    // 리소스 효율을 위해 요청이 하나 완료되면 그 때 WRITE 이벤트 등록
    if (!responsesMap[socket].empty()) {
        eventManager.addWriteEvent(socket);
    }

}

void Server::handleChunkedRequest(int socket, std::string& chunkedData) {
    size_t chunkLength = 0;
    bool isLastChunk = false;

    while (isCompleteChunk(chunkedData, chunkLength, isLastChunk)) {
        std::string chunk = chunkedData.substr(0, chunkLength);

        // NOTE: 파일 경로 수정해야 함
        ChunkedRequestReader reader("upload/testfile.png", chunk);
		bool isChunkedEnd = reader.processRequest();

        chunkedData.erase(0, chunkLength);

        if (isChunkedEnd) {
            isChunkedMap[socket] = false; // 마지막 청크 후 청크 상태 해제
            return ;
        }
    }
}

bool Server::isCompleteChunk(const std::string& data, size_t& chunkLength, bool& isLastChunk) {
    size_t pos = 0;
    size_t chunkSizeEnd = data.find("\r\n", pos);
    if (chunkSizeEnd == std::string::npos)
        return false; // 청크 크기가 아직 도착하지 않음

    size_t chunkSize = std::strtoul(data.substr(pos, chunkSizeEnd - pos).c_str(), NULL, 16);
    pos = chunkSizeEnd + 2; // 청크 크기 끝을 지나서 데이터 시작

    if (chunkSize == 0) {
        chunkLength = pos + 2; // 마지막 \r\n 포함
        if (data.size() >= chunkLength)
            isLastChunk = true; // 마지막 청크 처리
        return data.size() >= chunkLength;
    }

    if (pos + chunkSize + 2 > data.size())
        return false; // 청크 데이터가 아직 도착하지 않음
    chunkLength = pos + chunkSize + 2;
    return true;
}

void Server::handleClientWriteEvent(struct kevent& event)
{
    int socket = event.ident;
    std::vector<ResponseMessage*>& responses = responsesMap[socket];

    if (responses.empty()) {
        eventManager.deleteWriteEvent(socket);
        return ;
    }

    bool bKeepAlive = responses.back()->isKeepAlive();

    for (size_t i = 0; i < responses.size(); ++i) {
        sendResponse(socket, *responses[i]);
        delete responses[i];
    }
    responses.clear();

    eventManager.deleteWriteEvent(socket);

    if (!bKeepAlive) {
        closeConnection(socket);
    }
}

void Server::sendResponse(int socket, ResponseMessage& res)
{
    std::string responseStr = res.toString();

    if ((send(socket, responseStr.c_str(), responseStr.length(), 0)) < 0)
    {
        std::cerr << "send error: " << std::endl;
        closeConnection(socket);
        return;
    }
}

void Server::logHTTPMessage(int socket, const ResponseMessage& res, const std::string& reqData)
{
    std::ofstream logFile("access.log", std::ios::app);

    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);

    logFile << asctime(timeinfo) << "Client IP: " << socketToConfigMap[socket].host << '\n'
            << "Request:\n"
            << reqData << "Response:\n"
            << res.toString() << "\n----------------------------------------------------------------------------------------\n"
            << std::endl;
    logFile.close();
}

// 클라이언트 소켓 종료
void Server::closeConnection(Connection& connection)
{
    int fd = connection.getFd();
    eventManager.deleteReadEvent(fd);

    connection.close();

    connections.erase(fd);
}
