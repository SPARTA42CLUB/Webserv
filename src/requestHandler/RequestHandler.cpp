#include "RequestHandler.hpp"
#include <fcntl.h>
#include <ctime>
#include <fstream>
#include "EventManager.hpp"
#include "FileManager.hpp"
#include "SysException.hpp"

RequestHandler::RequestHandler(std::map<int, Connection*>& connectionsMap, const int fd)
: mConnectionsMap(connectionsMap)
, mSocket(fd)
, mRequestMessage(connectionsMap[fd]->request)
, mResponseMessage()
, mServerConfig(connectionsMap[fd]->serverConfig)
, mLocConfig()
, mPath()
, mQueryString()
, mbIsCGI(false)
{
}
ResponseMessage* RequestHandler::handleRequest(void)
{
    int statusCode = mConnectionsMap[mSocket]->request->getStatusCode();
    if (statusCode != OK)
    {
        mResponseMessage = new ResponseMessage();
        mResponseMessage->setByStatusCode(statusCode, mServerConfig);
        return mResponseMessage;
    }
    processRequestPath();
    try
    {
        if (mbIsCGI)
        {
            executeCGI();
            return NULL;
        }
    }
    catch (const SysException& e)
    {
        mResponseMessage = new ResponseMessage();
        mResponseMessage->setByStatusCode(SERVICE_UNAVAILABLE, mServerConfig);
        return mResponseMessage;
    }
    mResponseMessage = new ResponseMessage();

    statusCode = handleMethod();
    if (checkStatusCode(statusCode) == false)
    {
        mResponseMessage->setByStatusCode(statusCode, mServerConfig);
        return mResponseMessage;
    }

    return mResponseMessage;
}
void RequestHandler::processRequestPath(void)
{
    const std::string& reqTarget = mRequestMessage->getRequestLine().getRequestTarget();
    const std::map<std::string, LocationConfig>& locations = mServerConfig.locations;

    if (matchExactLocation(reqTarget, locations))
    {
        return;
    }
    matchClosestLocation(reqTarget, locations);
}
void RequestHandler::setPath(const std::map<std::string, LocationConfig>::const_iterator locIt, const std::string& reqTarget)
{
    mLocConfig = locIt->CONFIG;
    if (reqTarget.empty())
    {
        if (mLocConfig.alias.empty())
        {
            mPath = mLocConfig.root + locIt->LOCATION;
        }
        else
        {
            mPath = mLocConfig.alias;
        }
    }
    else
    {
        if (mLocConfig.alias.empty())
        {
            mPath = mLocConfig.root + reqTarget;
        }
        else
        {
            mPath = mLocConfig.alias + reqTarget.substr(locIt->LOCATION.size() - (locIt->LOCATION.back() == '/'));
        }
    }
}
bool RequestHandler::matchExactLocation(const std::string& reqTarget, const std::map<std::string, LocationConfig>& locations)
{
    std::map<std::string, LocationConfig>::const_iterator it = locations.find(reqTarget);
    if (it != locations.end())
    {
        setPath(it);
        return true;
    }

    if (reqTarget.back() == '/')
    {
        return false;
    }

    it = locations.find(reqTarget + "/");
    if (it != locations.end())
    {
        setPath(it);
        return true;
    }

    return false;
}
void RequestHandler::matchClosestLocation(const std::string& reqTarget, const std::map<std::string, LocationConfig>& locations)
{
    std::map<std::string, LocationConfig>::const_iterator tmp;
    size_t maxMatchCnt = 0;
    for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it)
    {
        if (it->LOCATION.front() == '.' && identifyCGIRequest(reqTarget, it))
        {
            mbIsCGI = true;
            return;
        }
        size_t matchCnt = 0;
        size_t trailingSlash = it->LOCATION.size() - (it->LOCATION.back() == '/');
        if (reqTarget.find(it->LOCATION) == 0 && reqTarget[trailingSlash] == '/')
        {
            matchCnt = it->LOCATION.size();
        }
        if (matchCnt > maxMatchCnt)
        {
            maxMatchCnt = matchCnt;
            tmp = it;
        }
    }
    setPath(tmp, reqTarget);
    return;
}
bool RequestHandler::identifyCGIRequest(const std::string& reqTarget, std::map<std::string, LocationConfig>::const_iterator& locIt)
{
    const std::string& locationPath = locIt->LOCATION;
    size_t dotIdx = reqTarget.find(locationPath);

    if (dotIdx == std::string::npos || reqTarget.substr(dotIdx, locationPath.size()) != locationPath)
    {
        return false;
    }

    size_t queryIdx = reqTarget.find('?');
    if ((queryIdx == std::string::npos && reqTarget.size() > dotIdx + locationPath.size()) ||
        (queryIdx != std::string::npos && queryIdx != dotIdx + locationPath.size()))
    {
        return false;
    }
    mLocConfig = locIt->CONFIG;
    mPath = (mLocConfig.alias.empty() ? mLocConfig.root : mLocConfig.alias) + reqTarget.substr(0, dotIdx + locationPath.size());
    if (queryIdx != std::string::npos)
    {
        mQueryString = reqTarget.substr(queryIdx + 1);
    }
    return true;
}
void RequestHandler::executeCGI(void)
{
    int pipe_in[2], pipe_out[2];
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
    {
        throw SysException(FAILED_TO_CREATE_PIPE);
    }
    pid_t pid = fork();
    if (pid == -1)
    {
        throw SysException(FAILED_TO_FORK);
    }
    if (pid == 0)
    {
        // Child process
        close(pipe_in[WRITE_END]);
        close(pipe_out[READ_END]);
        if (dup2(pipe_in[READ_END], STDIN_FILENO) == -1 || dup2(pipe_out[WRITE_END], STDOUT_FILENO) == -1)
            throw SysException(FAILED_TO_DUP);

        std::vector<char> interpreter_cstr(mLocConfig.cgi_interpreter.begin(), mLocConfig.cgi_interpreter.end());
        std::vector<char> path_cstr(mPath.begin(), mPath.end());
        interpreter_cstr.push_back('\0');
        path_cstr.push_back('\0');
        char* argv[] = {interpreter_cstr.data(), path_cstr.data(), NULL};

        std::string queryStringEnv = "QUERY_STRING=" + mQueryString;
        std::string requestMethodEnv = "REQUEST_METHOD=" + mRequestMessage->getRequestLine().getMethod();
        std::string contentLengthEnv = "CONTENT_LENGTH=" + std::to_string(mRequestMessage->getMessageBody().size());
        std::vector<char> queryStringEnv_cstr(queryStringEnv.begin(), queryStringEnv.end());
        std::vector<char> requestMethodEnv_cstr(requestMethodEnv.begin(), requestMethodEnv.end());
        std::vector<char> contentLengthEnv_cstr(contentLengthEnv.begin(), contentLengthEnv.end());
        queryStringEnv_cstr.push_back('\0');
        requestMethodEnv_cstr.push_back('\0');
        contentLengthEnv_cstr.push_back('\0');
        char* envp[] = {queryStringEnv_cstr.data(), requestMethodEnv_cstr.data(), contentLengthEnv_cstr.data(), NULL};

        execve(argv[READ_END], argv, envp);
        throw SysException(FAILED_TO_EXEC);
    }
    // Parent process
    close(pipe_in[READ_END]);
    close(pipe_out[WRITE_END]);

    fileManager::setNonBlocking(pipe_in[WRITE_END]);
    fileManager::setNonBlocking(pipe_out[READ_END]);

    Connection* parentConnection = mConnectionsMap[mSocket];
    parentConnection->cgiPid = pid;

    parentConnection->childFd[WRITE_END] = pipe_in[WRITE_END];
    parentConnection->childFd[READ_END] = pipe_out[READ_END];
    mConnectionsMap[pipe_in[WRITE_END]] = new Connection(pipe_in[WRITE_END], parentConnection->serverConfig, mSocket, mRequestMessage->getMessageBody().toString());
    mConnectionsMap[pipe_out[READ_END]] = new Connection(pipe_out[READ_END], parentConnection->serverConfig, mSocket);

    EventManager::getInstance().addWriteEvent(pipe_in[WRITE_END]);
}
// method에 따른 분기 처리
int RequestHandler::handleMethod(void)
{
    std::string method = mRequestMessage->getRequestLine().getMethod();

    if (mLocConfig.allow_methods.find(method) == mLocConfig.allow_methods.end())
    {
        return METHOD_NOT_ALLOWED;
    }

    if (!mLocConfig.redirect.empty())
    {
        return redirect();
    }
    else if (method == "GET")
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
int RequestHandler::redirect(void)
{
    mResponseMessage->addMessageBody("<html><head><title>302 Found</title></head><body><h1>302 Found</h1><p>This resource has been moved to <a href=\"" + mLocConfig.redirect + "\">" + mLocConfig.redirect + "</a>.</p></body></html>");
    mResponseMessage->setStatusLine("HTTP/1.1", FOUND, "Found");
    mResponseMessage->addResponseHeaderField("Location", mLocConfig.redirect);
    mResponseMessage->addResponseHeaderField("Connection", "close");
    mResponseMessage->addSemanticHeaderFields();
    return FOUND;
}
int RequestHandler::getRequest(void)
{
    handleIndex();
    int fStatus = fileManager::getFileStatus(mPath);
    if (fStatus == fileManager::DIRECTORY && mLocConfig.directory_listing)
        return handleAutoindex();

    if (fStatus == fileManager::NONE)
    {
        return NOT_FOUND;
    }

    std::ifstream file(mPath, std::ios::binary);
    if (!file.is_open() || fStatus != fileManager::FILE)
    {
        return FORBIDDEN;
    }

    std::ostringstream oss;
    oss << file.rdbuf();

    mResponseMessage->setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), OK, "OK");
    mResponseMessage->addMessageBody(oss.str());
    mResponseMessage->addSemanticHeaderFields();
    addContentType();

    file.close();
    return OK;
}
int RequestHandler::headRequest(void)
{
    int statusCode = getRequest();
    mResponseMessage->clearMessageBody();

    return statusCode;
}
int RequestHandler::postRequest(void)
{
    // NOTE: 파일에 대해 있다면 Append, 없다면 Create 후 바디를 쓴다.
    // 디렉토리 경로에 `Content-Disposition` 헤더 필드 안으로 filename이 있다면 그 파일명으로 저장

    // 디렉토리에 index 붙이기
    handleIndex();
    int fStatus = fileManager::getFileStatus(mPath);
    // 디렉토리가 아닐 때, 파일이 존재하면 파일 뒤에 데이터를 추가, 파일이 없으면 파일을 생성하고 데이터를 추가
    if (fStatus != fileManager::DIRECTORY)
    {
        return handleFileUpload(fStatus);
    }

    // 디렉토리인데, Content-Disposition 헤더 필드가 없으면 403 Forbidden
    const std::string filename = parseContentDisposition();
    if (filename.empty())
    {
        return FORBIDDEN;
    }

    // mPath의 뒤에 filename을 붙여서 파일 경로를 만든다.
    mPath.back() != '/' ? mPath += "/" : mPath;
    mPath += filename;

    fStatus = fileManager::getFileStatus(mPath);
    return handleFileUpload(fStatus);
}
int RequestHandler::deleteRequest()
{
    if (fileManager::isExist(mPath) == false)
    {
        return NOT_FOUND;
    }
    if (fileManager::deleteFile(mPath) == false)
    {
        return FORBIDDEN;
    }
    mResponseMessage->setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), OK, "OK");
    mResponseMessage->addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage->addMessageBody("<html><body><h1>File deleted.</h1></body></html>");
    mResponseMessage->addSemanticHeaderFields();

    return OK;
}
int RequestHandler::handleFileUpload(const int fStatus)
{
    std::ofstream file(mPath, std::ios::app | std::ios::binary);
    if (!file.is_open())
    {
        return FORBIDDEN;
    }
    file << mRequestMessage->getMessageBody().toString();
    file.close();
    if (fStatus == fileManager::FILE)
    {
        mResponseMessage->setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), OK, "OK");
        mResponseMessage->addMessageBody("<html><body><h1>File uploaded.</h1><h2>path: " + mPath + "</h2></body></html>");
    }
    else
    {
        mResponseMessage->setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), CREATED, "Created");
        mResponseMessage->addMessageBody("<html><body><h1>File created.</h1><h2>path: " + mPath + "</h2></body></html>");
    }
    mResponseMessage->addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage->addSemanticHeaderFields();
    return (fStatus == fileManager::FILE ? OK : CREATED);
}
std::string RequestHandler::parseContentDisposition(void)
{
    std::string filename = mRequestMessage->getRequestHeaderFields().getField("Content-Disposition");
    if (filename.empty())
    {
        filename = mRequestMessage->getRequestHeaderFields().getField("content-disposition");
    }
    if (filename.empty())
    {
        return "";
    }

    // Content-Disposition 헤더 필드에서 filename을 추출
    size_t idx = filename.find("filename=");
    if (idx == std::string::npos)
    {
        return "";
    }
    filename = filename.substr(idx + 10);
    idx = filename.find("\"");
    if (idx == std::string::npos)
    {
        return "";
    }
    filename = filename.substr(0, idx);
    return filename;
}
int RequestHandler::handleAutoindex()
{
    const std::string dirList = fileManager::listDirectoryContents(mPath, mLocConfig);
    if (dirList.empty())
    {
        return FORBIDDEN;
    }
    mResponseMessage->setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), OK, "OK");
    mResponseMessage->addMessageBody(dirList);
    mResponseMessage->addSemanticHeaderFields();
    return OK;
}
void RequestHandler::handleIndex()
{
    if (fileManager::getFileStatus(mPath) != fileManager::DIRECTORY || mLocConfig.index.empty())
    {
        return;
    }
    if (mPath.back() != '/')
    {
        mPath += "/";
    }
    mPath += mLocConfig.index;
}
void RequestHandler::addContentType(void)
{
    std::string ext = mPath.substr(mPath.find_last_of('.') + 1);
    if (ext == "html")
    {
        mResponseMessage->addResponseHeaderField("Content-Type", "text/html");
    }
    else if (ext == "css")
    {
        mResponseMessage->addResponseHeaderField("Content-Type", "text/css");
    }
    else if (ext == "js")
    {
        mResponseMessage->addResponseHeaderField("Content-Type", "text/javascript");
    }
    else if (ext == "txt")
    {
        mResponseMessage->addResponseHeaderField("Content-Type", "text/plain");
    }
    else if (ext == "jpg" || ext == "jpeg")
    {
        mResponseMessage->addResponseHeaderField("Content-Type", "image/jpeg");
    }
    else if (ext == "png")
    {
        mResponseMessage->addResponseHeaderField("Content-Type", "image/png");
    }
    else
    {
        mResponseMessage->addResponseHeaderField("Content-Type", "application/octet-stream");
    }
}
