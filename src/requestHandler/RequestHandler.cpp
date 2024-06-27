#include "RequestHandler.hpp"
#include "RangeRequestReader.hpp"
#include <unistd.h>
#include <ctime>
#include <fstream>

#include <iostream>

RequestHandler::RequestHandler(Connection& connection, const Config& config)
: mConnection(connection)
, mRequestMessage(connection.requests.front())
, mResponseMessage(new ResponseMessage())
, mServerConfig(config.getServerConfigByHost(mRequestMessage->getRequestHeaderFields().getField("Host")))
, mPath("")
{
}

/*
    // NOTE: 될 수 있는 조합
    GET
    1. URI == Location이며 URI가 디렉토리(/로 끝남)
        a. index 파일이 있을 경우 index 파일을 읽음
        b. index 파일이 없을 경우 404
        c. redirect가 있을 경우
            1. redirect <location> : 302(기본값) 반환, Location 헤더에 <Location> 추가 (nginx는 상태코드 추가 가능)
    2. URI == Location이며 URI가 파일
        a. 파일이 있을 경우 파일을 읽음
        b. 파일이 없을 경우 404
        x. redirect가 있을 경우

    3. URI != Location
        a. 모든 location 블록에서 root + index 파일을 찾음
    HEAD: GET과 동일하나 body가 없음
    POST
    DELETE
     */
ResponseMessage* RequestHandler::handleRequest(void)
{
    (void)mConnection;
    int statusCode = mRequestMessage->getStatusCode();

    // Request 에러면 미리 던지기. 내부에서 response 설정해줌
    if (checkStatusCode(statusCode) == false)
        return mResponseMessage;

    // CGI 부분 의사 코드
    // connection = new Connection()->parentConnection = mConnection;

    // request -> CGI -> CGIHandler(connection)
    // {
    //     connection.socket = pipe() : socket;
    //     EventManager::getInstance().addReadEvent(socket);

    //     fork()
    //     execve()
    // }

    // ConnectionMap[connection.socket] = connection;

    statusCode = setPath();
    if (checkStatusCode(statusCode) == false)
        return mResponseMessage;

    statusCode = handleMethod();
    if (checkStatusCode(statusCode) == false)
        return mResponseMessage;

    addConnectionHeader();

    return mResponseMessage;
}

int RequestHandler::setPath()
{
    std::string reqTarget = mRequestMessage->getRequestLine().getRequestTarget();
    std::map<std::string, LocationConfig>::const_iterator targetFindIter = mServerConfig.locations.find(reqTarget);

    // NOTE: 요청한 URI에 해당하는 LocationConfig 찾기
    // 만약 URI에 해당하는 Location이 있다면 해당 Location의 root 경로에 index 파일을 찾음
    if (targetFindIter == mServerConfig.locations.end())
    {
        for (std::map<std::string, LocationConfig>::const_iterator it = mServerConfig.locations.begin(); it != mServerConfig.locations.end(); it++)
        {
            if (it->first.back() == '/')
            {
                std::string tmp = it->second.root + reqTarget;
                if (access(tmp.c_str(), F_OK) == 0)
                {
                    mLocConfig = it->second;
                    mPath = tmp;
                    break;
                }
            }
        }
        if (mPath == "")
        {
            return NOT_FOUND;
        }
    }
    else
    {
        mLocConfig = targetFindIter->second;
        mPath = mLocConfig.root + reqTarget + mLocConfig.index;
    }

    return OK;
}

// method에 따른 분기 처리
int RequestHandler::handleMethod()
{
    std::string method = mRequestMessage->getRequestLine().getMethod();

    // NOTE: allow_methods 블록이 없을 경우 모든 메소드 허용
    if (mLocConfig.allow_methods.empty() == false)
    {
        if (mLocConfig.allow_methods.find(method) == mLocConfig.allow_methods.end())
        {
            return METHOD_NOT_ALLOWED;
        }
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
    if (file.is_open() == false)
    {
        return FORBIDDEN;
    }
	// Range 요청 판별
	if (mRequestMessage->getRequestHeaderFields().hasField("Range")) {
		std::string rangeHeader = mRequestMessage->getRequestHeaderFields().getField("Range");
		// Check if the header starts with "bytes="
		if (rangeHeader.substr(0, 6) == "bytes=") {
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
    addSemanticHeaderFields();
    addContentType();

    return OK;
}

int RequestHandler::rangeRequest()
{
	// Extract range values
	std::string rangeHeader = mRequestMessage->getRequestHeaderFields().getField("Range");
	std::vector<std::pair<size_t, size_t> > ranges; // 벡터 타입 명시적으로 지정

	// Remove "bytes=" from the header
	rangeHeader = rangeHeader.substr(6);

	// Split the header by commas to handle multiple ranges
	std::istringstream iss(rangeHeader);
	std::string rangePart;
	while (std::getline(iss, rangePart, ',')) {
		size_t dashPos = rangePart.find('-');
		if (dashPos != std::string::npos) {
			size_t rangeStart = std::stoul(rangePart.substr(0, dashPos));
			size_t rangeEnd = std::stoul(rangePart.substr(dashPos + 1));
			ranges.push_back(std::make_pair(rangeStart, rangeEnd));
		}
	}

	// Create a RangeRequestReader instance
	RangeRequestReader reader(mPath);

	// Add all ranges to the reader
	for (std::vector<std::pair<size_t, size_t> >::const_iterator it = ranges.begin(); it != ranges.end(); ++it) {
		reader.addRange((*it).first, (*it).second); // 반복자 사용하여 요소 접근
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
    mResponseMessage->setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), OK, "OK");
    mResponseMessage->addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage->addMessageBody("<html><body><h1>POST Request</h1></body></html>");
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
    mResponseMessage->setStatusLine(mRequestMessage->getRequestLine().getHTTPVersion(), OK, "OK");
    mResponseMessage->addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage->addMessageBody("<html><body><h1>File deleted.</h1></body></html>");
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
    // mServerConfig.locations.find(mLocation)->second.cgi;

    mResponseMessage->addResponseHeaderField("Content-Length", mResponseMessage->getMessageBodySize());
    mResponseMessage->addResponseHeaderField("Server", "webserv");
    mResponseMessage->addResponseHeaderField("Date", date);
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
    mResponseMessage->setStatusLine("HTTP/1.1", FOUND, "Found");
    mResponseMessage->addResponseHeaderField("Location", location);
    mResponseMessage->addResponseHeaderField("Connection", "close");
    addSemanticHeaderFields();
}
void RequestHandler::badRequest(void)
{
    mResponseMessage->setStatusLine("HTTP/1.1", BAD_REQUEST, "Bad Request");
    mResponseMessage->addMessageBody("<html><head><title>400 Bad Request</title></head><body><h1>400 Bad Request</h1></body></html>");
    mResponseMessage->addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage->addResponseHeaderField("Connection", "close");
    addSemanticHeaderFields();
}
void RequestHandler::forbidden(void)
{
    mResponseMessage->setStatusLine("HTTP/1.1", FORBIDDEN, "Forbidden");
    mResponseMessage->addMessageBody("<html><head><title>403 Forbidden</title></head><body><h1>403 Forbidden</h1></body></html>");
    mResponseMessage->addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage->addResponseHeaderField("Connection", "keep-alive");
    addSemanticHeaderFields();
}
void RequestHandler::notFound(void)
{
    mResponseMessage->setStatusLine("HTTP/1.1", NOT_FOUND, "Not Found");
    mResponseMessage->addMessageBody("<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>");
    mResponseMessage->addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage->addResponseHeaderField("Connection", "keep-alive");
    addSemanticHeaderFields();
}
void RequestHandler::methodNotAllowed(void)
{
    mResponseMessage->setStatusLine("HTTP/1.1", METHOD_NOT_ALLOWED, "Method Not Allowed");
    mResponseMessage->addMessageBody("<html><head><title>405 Method Not Allowed</title></head><body><h1>405 Method Not Allowed</h1></body></html>");
    mResponseMessage->addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage->addResponseHeaderField("Connection", "keep-alive");
    addSemanticHeaderFields();
}
void RequestHandler::uriTooLong(void)
{
    mResponseMessage->setStatusLine("HTTP/1.1", URI_TOO_LONG, "Request-URI Too Long");
    mResponseMessage->addMessageBody("<html><head><title>414 Request-URI Too Long</title></head><body><h1>414 Request-URI Too Long</h1></body></html>");
    mResponseMessage->addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage->addResponseHeaderField("Connection", "keep-alive");
    addSemanticHeaderFields();
}
void RequestHandler::httpVersionNotSupported(void)
{
    mResponseMessage->setStatusLine("HTTP/1.1", HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported");
    mResponseMessage->addMessageBody("<html><head><title>505 HTTP Version Not Supported</title></head><body><h1>505 HTTP Version Not Supported</h1></body></html>");
    mResponseMessage->addResponseHeaderField("Content-Type", "text/html");
    mResponseMessage->addResponseHeaderField("Connection", "keep-alive");
    addSemanticHeaderFields();
}
