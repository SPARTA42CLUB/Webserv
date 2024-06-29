#include "RequestHandler.hpp"
#include <ctime>
#include <fstream>
#include <fcntl.h>
#include "RangeRequestReader.hpp"
#include "EventManager.hpp"
#include "SysException.hpp" // TODO: 에러 이걸로 던짐?????
#include "FileChecker.hpp"
#include "fileController.hpp"

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

ResponseMessage* RequestHandler::handleRequest(void)
{
    int statusCode;

    if (checkCGI())
    {
        // std::cout << "path: " << mPath << std::endl;
        // std::cout << "query: " << mQueryString << std::endl;
        executeCGI();
        return NULL;
    }
    statusCode = setPath();
    // std::cout << mPath << std::endl;

    mResponseMessage = new ResponseMessage();
    if (checkStatusCode(statusCode) == false)
    {
        mResponseMessage->setByStatusCode(statusCode, mServerConfig);
        return mResponseMessage;
    }

    statusCode = handleMethod();
    if (checkStatusCode(statusCode) == false)
    {
        mResponseMessage->setByStatusCode(statusCode, mServerConfig);
        return mResponseMessage;
    }

    addConnectionHeader();

    return mResponseMessage;
}
bool RequestHandler::checkCGI(void)
{
    const std::string& reqTarget = mRequestMessage->getRequestLine().getRequestTarget();
    const std::map<std::string, LocationConfig>& locations = mServerConfig.locations;

    for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it)
    {
        if (it->LOCATION.front() == '.')
        {
            size_t idx = reqTarget.find(it->LOCATION);
            if (idx == std::string::npos)
                continue;
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
        throw SysException(FAILED_TO_CREATE_PIPE);
    }
	pid_t pid = fork();
	if(pid == -1)
    {
        throw SysException(FAILED_TO_FORK);
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
		throw SysException(FAILED_TO_EXEC);
	}
	else
	{
        // Parent process
        close(pipe_in[READ_END]);
        close(pipe_out[WRITE_END]);

        setNonBlocking(pipe_in[WRITE_END]);
        setNonBlocking(pipe_out[READ_END]);

        mConnectionsMap[mSocket]->childSocket[WRITE_END] = pipe_in[WRITE_END];
        mConnectionsMap[mSocket]->childSocket[READ_END] = pipe_out[READ_END];
        mConnectionsMap[pipe_in[WRITE_END]] = new Connection(pipe_in[WRITE_END], mSocket, mRequestMessage->getMessageBody().toString());
        mConnectionsMap[pipe_out[READ_END]] = new Connection(pipe_out[READ_END], mSocket);

        // write(pipe_in[1], mRequestMessage->getMessageBody().toString().c_str(), mRequestMessage->getMessageBody().size());
        // wait(NULL);
        // char buf[4096];
        // read(pipe_out[READ_END], buf, sizeof(buf));
        // std::cout << buf << std::endl;
		EventManager::getInstance().addWriteEvent(pipe_in[WRITE_END]);
        // EventManager::getInstance().addReadEvent(pipe_out[READ_END]);

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
    if (mPath.back() == '/')
        mPath += mLocConfig.index;
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

	// TODO(6/29 13:10): 여기서 Redirect 처리

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
        mResponseMessage->addResponseHeaderField("Connection", mRequestMessage->getRequestHeaderFields().getField("Connection"));
    }
    else
    {
        mResponseMessage->addResponseHeaderField("Connection", "keep-alive");
    }
}

int RequestHandler::getRequest()
{
    std::ifstream file(mPath);
    if (access(mPath.c_str(), F_OK) != 0)
    {
        return NOT_FOUND;
    }
    if (!file.is_open())
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
    mResponseMessage->setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), OK, "OK");
    std::string line;
    while (std::getline(file, line))
    {
        // TODO: 파일 마지막에 개행이 없는 경우 개행이 추가로 들어가는 문제
        mResponseMessage->addMessageBody(line + "\n");
    }
    file.close();
    mResponseMessage->addSemanticHeaderFields();
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
    mResponseMessage->setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), PARTIAL_CONTENT, "Partial Content");
    mResponseMessage->addResponseHeaderField("Content-Type", "multipart/byteranges; boundary=BOUNDARY_STRING");
    mResponseMessage->addMessageBody(responseBody);

    return OK;
}

int RequestHandler::headRequest()
{
    int statusCode = getRequest();
    mResponseMessage->clearMessageBody();

    return statusCode;
}
int RequestHandler::postRequest()
{
	// Content-Disposition 헤더 필드가 없으면 400 Bad Request
	if (mRequestMessage->getRequestHeaderFields().hasField("Content-Disposition") == false && mRequestMessage->getRequestHeaderFields().hasField("content-disposition") == false )
	{
		return BAD_REQUEST;
	}

	// Content-Disposition 헤더 필드에서 filename을 추출
	std::string filename;
	if (mRequestMessage->getRequestHeaderFields().hasField("Content-Disposition"))
	{
		filename = mRequestMessage->getRequestHeaderFields().getField("Content-Disposition");
	}
	else
	{
		filename = mRequestMessage->getRequestHeaderFields().getField("content-disposition");
	}
	size_t idx = filename.find("filename=");
	if (idx == std::string::npos)
	{
		return BAD_REQUEST;
	}
	filename = filename.substr(idx + 10);
	idx = filename.find("\"");
	if (idx == std::string::npos)
	{
		return BAD_REQUEST;
	}
	filename = filename.substr(0, idx);

	// mPath의 뒤에 filename을 붙여서 파일 경로를 만든다.
	if (mPath.back() != '/')
	{
		mPath += '/';
	}
	mPath += filename;

	// 파일이 존재하면 파일 뒤에 데이터를 추가하고, 파일이 없으면 파일을 생성하고 데이터를 추가함
	if (access(mPath.c_str(), F_OK) != 0)
	{
		std::ofstream file(mPath);
		if (!file.is_open())
		{
			return FORBIDDEN;
		}
		file << mRequestMessage->getMessageBody().toString();
		file.close();
	}
	else
	{
		std::ofstream file(mPath, std::ios::app);
		if (!file.is_open())
		{
			return FORBIDDEN;
		}
		file << mRequestMessage->getMessageBody().toString();
		file.close();
	}

	mResponseMessage->setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), CREATED, "CREATED");
	mResponseMessage->addResponseHeaderField("Content-Type", "text/html");
	mResponseMessage->addMessageBody("<html><body><h1>File uploaded.</h1><h2>path: " + mPath + "</h2></h3></body></html>");

    return CREATED;
}

// https://www.rfc-editor.org/rfc/rfc9110.html#name-delete
int RequestHandler::deleteRequest()
{
    if (access(mPath.c_str(), F_OK) != 0)
    {
        return NOT_FOUND;
    }
    if (std::remove(mPath.c_str()) != 0)
    {
        return FORBIDDEN;
    }
    mResponseMessage->setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), OK, "OK");
    mResponseMessage->addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage->addMessageBody("<html><body><h1>File deleted.</h1></body></html>");
    mResponseMessage->addSemanticHeaderFields();

    return OK;
}
void RequestHandler::addContentType()
{
    // NOTE: Config에 MIME 타입 추가
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

void RequestHandler::found(void)
{
    const std::string& location = mServerConfig.locations.find(mPath)->second.redirect;
    mResponseMessage->setStatusLine("HTTP/1.1", FOUND, "Found");
    mResponseMessage->addResponseHeaderField("Location", location);
    mResponseMessage->addResponseHeaderField("Connection", "close");
    mResponseMessage->addSemanticHeaderFields();
}
