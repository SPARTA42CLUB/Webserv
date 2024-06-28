#include "RequestHandler.hpp"
#include <ctime>
#include <fstream>
#include "RangeRequestReader.hpp"
#include "EventManager.hpp"
#include "SysException.hpp" // TODO: 에러 이걸로 던짐?????
#include "FileChecker.hpp"

// DELETE: 테스트용
#include <iostream>
RequestHandler::RequestHandler(std::map<int, Connection*>& connectionsMap, const Config& config, const int socket)
: mConnectionsMap(connectionsMap)
, mSocket(socket)
, mRequestMessage(connectionsMap[socket]->requests.front())
, mResponseMessage()
, mServerConfig(config.getServerConfigByHost(mRequestMessage->getRequestHeaderFields().getField("Host")))
, mLocConfig()
, mPath()
, mQueryString()
{
}

std::string RequestHandler::handleRequest(void)
{
    int statusCode = mRequestMessage->getStatusCode();
    // Request 에러면 미리 던지기. 내부에서 response 설정해줌
    if (checkStatusCode(statusCode) == false)
        return mResponseMessage.toString();

    if (checkCGI())
    {
        // std::cout << "path: " << mPath << std::endl;
        // std::cout << "query: " << mQueryString << std::endl;
        executeCGI();
        return "";
    }
    statusCode = setPath();
    std::cout << mPath << std::endl;

    if (checkStatusCode(statusCode) == false)
        return mResponseMessage.toString();

    statusCode = handleMethod();
    if (checkStatusCode(statusCode) == false)
        return mResponseMessage.toString();

    addConnectionHeader();

    return mResponseMessage.toString();
}
bool RequestHandler::checkCGI(void)
{
    const std::string& reqTarget = mRequestMessage->getRequestLine().getRequestTarget();
    const std::map<std::string, LocationConfig>& locations = mServerConfig.locations;

    if (reqTarget.find('.') == std::string::npos)
    {
        return false;
    }
    for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it)
    {
        if (it->LOCATION.front() == '.')
        {
            size_t idx = reqTarget.find(it->LOCATION);
            size_t query_idx = reqTarget.find('?');
            if (reqTarget.substr(idx, it->LOCATION.size()) == it->LOCATION)
            {
                mLocConfig = it->CONFIG;
                mPath = mLocConfig.root + reqTarget.substr(0, idx + it->LOCATION.size());
                if (query_idx == std::string::npos)
                {
                    return true;
                }
                mQueryString = reqTarget.substr(query_idx + 1);
                return true;
            }
            
        }
    }
    return false;
}
void RequestHandler::executeCGI(void)
{
    // extract query string from request body
	int pipe_in[2], pipe_out[2];
	if(pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
    {
        // throw SysException(FAILED_TO_CREATE_PIPE);
    }
	pid_t pid = fork();
	if(pid == -1)
    {
        // throw SysException(FAILED_TO_FORK);
    }
	if (pid == 0) 
    {
        // Child process
        close(pipe_in[WRITE_END]);
        close(pipe_out[READ_END]);
        dup2(pipe_in[READ_END], STDIN_FILENO);
        dup2(pipe_out[WRITE_END], STDOUT_FILENO);

        std::vector<char> interpreter_cstr(mLocConfig.cgi_interpreter.begin(), mLocConfig.cgi_interpreter.end());
        std::vector<char> path_cstr(mPath.begin(), mPath.end());
        interpreter_cstr.push_back('\0');
        path_cstr.push_back('\0');
		char* argv[] = {interpreter_cstr.data(), path_cstr.data(), NULL};

        std::string queryStringEnv = "QUERY_STRING=" + mQueryString;
        std::string requestMethodEnv = "REQUEST_METHOD=" + mRequestMessage->getRequestLine().getMethod();
        std::vector<char> queryStringEnv_cstr(queryStringEnv.begin(), queryStringEnv.end());
        std::vector<char> requestMethodEnv_cstr(requestMethodEnv.begin(), requestMethodEnv.end());
        queryStringEnv_cstr.push_back('\0');
        requestMethodEnv_cstr.push_back('\0');
		char* envp[] = {queryStringEnv_cstr.data(), requestMethodEnv_cstr.data(), NULL};

		execve(argv[READ_END], argv, envp);
		// throw SysException(FAILED_TO_EXEC);
	}
	else
	{
        // Parent process
        close(pipe_in[READ_END]);
        close(pipe_out[WRITE_END]);
        
        mConnectionsMap[mSocket]->childSocket[READ_END] = pipe_out[READ_END];
        mConnectionsMap[mSocket]->childSocket[WRITE_END] = pipe_in[WRITE_END];
        mConnectionsMap[pipe_in[WRITE_END]] = new Connection(pipe_in[WRITE_END], mSocket, mRequestMessage->getMessageBody().toString());
        mConnectionsMap[pipe_out[READ_END]] = new Connection(pipe_out[READ_END], mSocket);

        // write(pipe_in[1], mRequestMessage->getMessageBody().toString().c_str(), mRequestMessage->getMessageBody().size());
        // wait(NULL);
        // char buf[4096];
        // read(pipe_out[READ_END], buf, sizeof(buf));
        // std::cout << buf << std::endl;
		EventManager::getInstance().addWriteEvent(pipe_in[WRITE_END]);

        // https://codetravel.tistory.com/42
		// waitpid(pid, NULL, WNOHANG);
	}
}

size_t countMatchingCharacters(const std::string& target, const std::string& loc)
{
    if (target.find(loc) == 0)
    {
        return loc.size();
    }
    return 0;
}
int RequestHandler::setPath()
{
    const std::string& reqTarget = mRequestMessage->getRequestLine().getRequestTarget();
    const std::map<std::string, LocationConfig>& locations = mServerConfig.locations;
    std::map<std::string, LocationConfig>::const_iterator it = locations.find(reqTarget);

    if (locations.size() == 0)
    {
        mPath = mServerConfig.root + reqTarget;
    }
    else if (it != locations.end())
    {
        mLocConfig = it->CONFIG;
        mPath = mLocConfig.root + it->LOCATION;
    }
    else
    {
        const LocationConfig* locConf;
        size_t maxMatch = 0;
        for (it = locations.begin(); it != locations.end(); ++it)
        {
            if (it->LOCATION.back() != '/')
                continue;
            size_t matchCnt = countMatchingCharacters(reqTarget, it->LOCATION);
            if (matchCnt > maxMatch)
            {
                maxMatch = matchCnt;
                locConf = &(it->CONFIG);
            }
        }
        if (maxMatch == 0)
        {
            return NOT_FOUND;
        }
        mPath = locConf->root + reqTarget;
        mLocConfig = *locConf;
    }
    if (FileChecker::getFileStatus(mPath) == FileChecker::DIRECTORY)
    {
        mPath += mLocConfig.index;
    }
    if (FileChecker::getFileStatus(mPath) == FileChecker::NONE)
    {
        return NOT_FOUND;
    }

    return OK;
}

// method에 따른 분기 처리
int RequestHandler::handleMethod()
{
    std::string method = mRequestMessage->getRequestLine().getMethod();

    if (mLocConfig.allow_methods.find(method) == mLocConfig.allow_methods.end())
    {
        return METHOD_NOT_ALLOWED;
    }

    if (method == "GET")
    {
        return getRequest();
    }
    else if (method == "HEAD")
    {
        return headRequest();
    }
    else if (method == "POST")
    {
        return postRequest();
    }
    else if (method == "DELETE")
    {
        return deleteRequest();
    }
    else
    {
        return METHOD_NOT_ALLOWED;
    }
}

// Connection 헤더 필드 추가
void RequestHandler::addConnectionHeader()
{
    if (mRequestMessage->getRequestHeaderFields().hasField("Connection") == true)
    {
        mResponseMessage.addResponseHeaderField("Connection", mRequestMessage->getRequestHeaderFields().getField("Connection"));
    }
    else
    {
        mResponseMessage.addResponseHeaderField("Connection", "keep-alive");
    }
}

int RequestHandler::getRequest()
{
    std::ifstream file(mPath);
    if (!file.good())
    {
        return FORBIDDEN;
    }
    // Range 요청 판별
    if (mRequestMessage->getRequestHeaderFields().hasField("Range"))
    {
        std::string rangeHeader = mRequestMessage->getRequestHeaderFields().getField("Range");
        // Check if the header starts with "bytes="
        if (rangeHeader.substr(0, 6) == "bytes=")
        {
            file.close();
            return rangeRequest();
        }
    }
    // 그 외 요청 처리
    mResponseMessage.setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), OK, "OK");
    std::string line;
    while (std::getline(file, line))
    {
        // TODO: 파일 마지막에 개행이 없는 경우 개행이 추가로 들어가는 문제
        mResponseMessage.addMessageBody(line + "\n");
    }
    file.close();
    addSemanticHeaderFields();
    addContentType();

    return OK;
}

int RequestHandler::rangeRequest()
{
    // Extract range values
    std::string rangeHeader = mRequestMessage->getRequestHeaderFields().getField("Range");
    std::vector<std::pair<size_t, size_t> > ranges;  // 벡터 타입 명시적으로 지정

    // Remove "bytes=" from the header
    rangeHeader = rangeHeader.substr(6);

    // Split the header by commas to handle multiple ranges
    std::istringstream iss(rangeHeader);
    std::string rangePart;
    while (std::getline(iss, rangePart, ','))
    {
        size_t dashPos = rangePart.find('-');
        if (dashPos != std::string::npos)
        {
            size_t rangeStart = std::stoul(rangePart.substr(0, dashPos));
            size_t rangeEnd = std::stoul(rangePart.substr(dashPos + 1));
            ranges.push_back(std::make_pair(rangeStart, rangeEnd));
        }
    }

    // Create a RangeRequestReader instance
    RangeRequestReader reader(mPath);

    // Add all ranges to the reader
    for (std::vector<std::pair<size_t, size_t> >::const_iterator it = ranges.begin(); it != ranges.end(); ++it)
    {
        reader.addRange(it->LOCATION, it->CONFIG);  // 반복자 사용하여 요소 접근
    }

    // Process the request and get the response body
    std::string responseBody = reader.processRequest();

    // Set HTTP response status line and headers
    mResponseMessage.setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), PARTIAL_CONTENT, "Partial Content");
    mResponseMessage.addResponseHeaderField("Content-Type", "multipart/byteranges; boundary=BOUNDARY_STRING");
    mResponseMessage.addMessageBody(responseBody);

    return OK;
}

int RequestHandler::headRequest()
{
    int statusCode = getRequest();
    mResponseMessage.clearMessageBody();

    return statusCode;
}
int RequestHandler::postRequest()
{
    mResponseMessage.setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), OK, "OK");
    mResponseMessage.addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage.addMessageBody("<html><body><h1>POST Request</h1></body></html>");
    addSemanticHeaderFields();

    return OK;
}
// https://www.rfc-editor.org/rfc/rfc9110.html#name-delete
int RequestHandler::deleteRequest()
{
    if (mPath == "")
    {
        return NOT_FOUND;
    }
    if (std::remove(mPath.c_str()) != 0)
    {
        return METHOD_NOT_ALLOWED;
    }
    mResponseMessage.setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), OK, "OK");
    mResponseMessage.addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage.addMessageBody("<html><body><h1>File deleted.</h1></body></html>");
    addSemanticHeaderFields();

    return OK;
}

void RequestHandler::addSemanticHeaderFields()
{
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[128];
    // Date: Tue, 15 Nov 1994 08:12:31 GMT
    // NOTE: strftime() 함수는 c함수라서 평가에 어긋남, 하지만 Date 헤더 필드가 필수가 아니라서 보너스 느낌으로 넣어둠
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    std::string date = buffer;

    // NOTE: expression result unused
    // mServerConfig.locations.find(mLocation)->CONFIG.cgi;

    mResponseMessage.addResponseHeaderField("Content-Length", mResponseMessage.getMessageBodySize());
    mResponseMessage.addResponseHeaderField("Server", "webserv");
    mResponseMessage.addResponseHeaderField("Date", date);
}
void RequestHandler::addContentType()
{
    // NOTE: Config에 MIME 타입 추가
    std::string ext = mPath.substr(mPath.find_last_of('.') + 1);
    if (ext == "html")
    {
        mResponseMessage.addResponseHeaderField("Content-Type", "text/html");
    }
    else if (ext == "css")
    {
        mResponseMessage.addResponseHeaderField("Content-Type", "text/css");
    }
    else if (ext == "js")
    {
        mResponseMessage.addResponseHeaderField("Content-Type", "text/javascript");
    }
    else if (ext == "jpg" || ext == "jpeg")
    {
        mResponseMessage.addResponseHeaderField("Content-Type", "image/jpeg");
    }
    else if (ext == "png")
    {
        mResponseMessage.addResponseHeaderField("Content-Type", "image/png");
    }
    else
    {
        mResponseMessage.addResponseHeaderField("Content-Type", "application/octet-stream");
    }
}
bool RequestHandler::checkStatusCode(const int statusCode)
{
    if (statusCode == BAD_REQUEST)
    {
        badRequest();
        return false;
    }
    else if (statusCode == FORBIDDEN)
    {
        forbidden();
        return false;
    }
    else if (statusCode == NOT_FOUND)
    {
        notFound();
        return false;
    }
    else if (statusCode == METHOD_NOT_ALLOWED)
    {
        methodNotAllowed();
        return false;
    }
    else if (statusCode == URI_TOO_LONG)
    {
        uriTooLong();
        return false;
    }
    else if (statusCode == HTTP_VERSION_NOT_SUPPORTED)
    {
        httpVersionNotSupported();
        return false;
    }
    return true;
}
void RequestHandler::found(void)
{
    const std::string& location = mServerConfig.locations.find(mPath)->second.redirect;
    mResponseMessage.setStatusLine("HTTP/1.1", FOUND, "Found");
    mResponseMessage.addResponseHeaderField("Location", location);
    mResponseMessage.addResponseHeaderField("Connection", "close");
    addSemanticHeaderFields();
}
void RequestHandler::badRequest(void)
{
    mResponseMessage.setStatusLine("HTTP/1.1", BAD_REQUEST, "Bad Request");
    mResponseMessage.addMessageBody("<html><head><title>400 Bad Request</title></head><body><h1>400 Bad Request</h1></body></html>");
    mResponseMessage.addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage.addResponseHeaderField("Connection", "close");
    addSemanticHeaderFields();
}
void RequestHandler::forbidden(void)
{
    mResponseMessage.setStatusLine("HTTP/1.1", FORBIDDEN, "Forbidden");
    mResponseMessage.addMessageBody("<html><head><title>403 Forbidden</title></head><body><h1>403 Forbidden</h1></body></html>");
    mResponseMessage.addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage.addResponseHeaderField("Connection", "keep-alive");
    addSemanticHeaderFields();
}
void RequestHandler::notFound(void)
{
    mResponseMessage.setStatusLine("HTTP/1.1", NOT_FOUND, "Not Found");
    mResponseMessage.addMessageBody("<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>");
    mResponseMessage.addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage.addResponseHeaderField("Connection", "keep-alive");
    addSemanticHeaderFields();
}
void RequestHandler::methodNotAllowed(void)
{
    mResponseMessage.setStatusLine("HTTP/1.1", METHOD_NOT_ALLOWED, "Method Not Allowed");
    mResponseMessage.addMessageBody("<html><head><title>405 Method Not Allowed</title></head><body><h1>405 Method Not Allowed</h1></body></html>");
    mResponseMessage.addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage.addResponseHeaderField("Connection", "keep-alive");
    addSemanticHeaderFields();
}
void RequestHandler::uriTooLong(void)
{
    mResponseMessage.setStatusLine("HTTP/1.1", URI_TOO_LONG, "Request-URI Too Long");
    mResponseMessage.addMessageBody("<html><head><title>414 Request-URI Too Long</title></head><body><h1>414 Request-URI Too Long</h1></body></html>");
    mResponseMessage.addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage.addResponseHeaderField("Connection", "keep-alive");
    addSemanticHeaderFields();
}
void RequestHandler::httpVersionNotSupported(void)
{
    mResponseMessage.setStatusLine("HTTP/1.1", HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported");
    mResponseMessage.addMessageBody(
        "<html><head><title>505 HTTP Version Not Supported</title></head><body><h1>505 HTTP Version Not Supported</h1></body></html>");
    mResponseMessage.addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage.addResponseHeaderField("Connection", "keep-alive");
    addSemanticHeaderFields();
}
