#include "RequestHandler.hpp"
#include <unistd.h>
#include <ctime>
#include <fstream>
#include "ChunkedRequestReader.hpp"

RequestHandler::RequestHandler()
	: processingChunkedRequest(false)
{
}

void RequestHandler::verifyRequest(const RequestMessage& req, const ServerConfig& serverConfig)
{
    (void)serverConfig;
    try
    {
        verifyRequestLine(req.getRequestLine());
        verifyRequestHeaderFields(req.getRequestHeaderFields());
    }
    catch (const HTTPException& e)
    {
        throw e;
    }
}
void RequestHandler::verifyRequestLine(const RequestLine& reqLine)
{
    const std::string method = reqLine.getMethod();
    const std::string reqTarget = reqLine.getRequestTarget();
    const std::string ver = reqLine.getHTTPVersion();
    if (method != "GET" && method != "HEAD" && method != "POST" && method != "DELETE")
    {
        throw HTTPException(METHOD_NOT_ALLOWED);
    }
    if (ver != "HTTP/1.1")
    {
        throw HTTPException(HTTP_VERSION_NOT_SUPPORTED);
    }
    if (reqTarget[0] != '/')
    {
        throw HTTPException(NOT_FOUND);
    }
    if (reqTarget.size() >= 8200)  // nginx max uri length
    {
        throw HTTPException(URI_TOO_LONG);
    }
}
void RequestHandler::verifyRequestHeaderFields(const HeaderFields& reqHeaderFields)
{
    if (reqHeaderFields.hasField("Host") == false)
    {
        throw HTTPException(BAD_REQUEST);
    }
}
void RequestHandler::handleRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig)
{
    std::string reqTarget = req.getRequestLine().getRequestTarget();
    std::string method = req.getRequestLine().getMethod();
    std::string path = "";

    // NOTE: 요청한 URI에 해당하는 LocationConfig 찾기
    std::map<std::string, LocationConfig>::const_iterator targetFindIter = serverConfig.locations.find(reqTarget);
    if (targetFindIter == serverConfig.locations.end())
    {
        for (std::map<std::string, LocationConfig>::const_iterator it = serverConfig.locations.begin(); it != serverConfig.locations.end(); it++)
        {
            if (it->first.back() == '/')
            {
                std::string tmp = it->second.root + req.getRequestLine().getRequestTarget();
                if (access(tmp.c_str(), F_OK) == 0)
                {
                    path = tmp;
                    break;
                }
            }
        }
        if (path == "")
        {
            throw HTTPException(NOT_FOUND);
        }
    }
    else
    {
        path = targetFindIter->second.root + req.getRequestLine().getRequestTarget() + targetFindIter->second.index;
    }

    // NOTE: allow_methods 블록이 없을 경우 모든 메소드 허용
    // WARNING: segment fault 발생
    /*
    ==12581==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x000107b03567 at pc 0x0001043c3520 bp 0x00016ba486b0 sp 0x00016ba486a8
    READ of size 1 at 0x000107b03567 thread T0
    #0 0x1043c351c in std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>::__is_long[abi:ue170006]() const string:1734
    #1 0x1043ba13c in std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>::size[abi:ue170006]() const string:1168
    #2 0x1043c87a0 in bool std::__1::operator==[abi:ue170006]<std::__1::allocator<char>>(std::__1::basic_string<char, std::__1::char_traits<char>,
    std::__1::allocator<char>> const&, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&) string:3897 #3 0x1043c7ae4
    in std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*
    std::__1::__find_impl[abi:ue170006]<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*,
    std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*, std::__1::basic_string<char, std::__1::char_traits<char>,
    std::__1::allocator<char>>, std::__1::__identity>(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*,
    std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*, std::__1::basic_string<char, std::__1::char_traits<char>,
    std::__1::allocator<char>> const&, std::__1::__identity&) find.h:34 #4 0x1043baadc in std::__1::__wrap_iter<std::__1::basic_string<char,
    std::__1::char_traits<char>, std::__1::allocator<char>> const*> std::__1::find[abi:ue170006]<std::__1::__wrap_iter<std::__1::basic_string<char,
    std::__1::char_traits<char>, std::__1::allocator<char>> const*>, std::__1::basic_string<char, std::__1::char_traits<char>,
    std::__1::allocator<char>>>(std::__1::__wrap_iter<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*>,
    std::__1::__wrap_iter<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*>, std::__1::basic_string<char,
    std::__1::char_traits<char>, std::__1::allocator<char>> const&) find.h:81 #5 0x1043b8ffc in RequestHandler::handleRequest(RequestMessage const&,
    ResponseMessage&, ServerConfig const&) RequestHandler.cpp:35 #6 0x1043e2890 in Server::handleClientReadEvent(kevent&) Server.cpp:192 #7 0x1043e03f8 in
    Server::run() Server.cpp:111 #8 0x10440f528 in main main.cpp:18 #9 0x18351a0dc  (<unknown module>)
     */
    // const LocationConfig& locConfig = targetFindIter->second;
    // if (std::find(locConfig.allow_methods.begin(), locConfig.allow_methods.end(), method) == locConfig.allow_methods.end())
    // {
    //     throw HTTPException(METHOD_NOT_ALLOWED);
    // }

    // Connection 헤더 필드 추가
    if (req.getRequestHeaderFields().hasField("Connection") == true)
    {
        res.addResponseHeaderField("Connection", req.getRequestHeaderFields().getField("Connection"));
    }
    else
    {
        res.addResponseHeaderField("Connection", "close");
    }

    // Method에 따른 처리
    try
    {
        if (method == "GET")
        {
            getRequest(req, res, serverConfig, path);
        }
        else if (method == "HEAD")
        {
            headRequest(req, res, serverConfig, path);
        }
        else if (method == "POST")
        {
			if (this->processingChunkedRequest)
			{
				// std::cout << "Processing chunked request" << std::endl;
				// std::cout << req.getMessageBody().toString() << std::endl;
				std::cout << "-----------message body-----------\n";
				std::cout << req.getMessageBody().size() << std::endl;
				std::cout << req.getMessageBody().toString() << std::endl;
				std::cout << "----------------------------------\n";
				ChunkedRequestReader reader("var/www/upload/files/testfile.png", req.getMessageBody().toString());
				if (reader.processRequest()) {
					processingChunkedRequest = false;
				}
			}
			else
			{
				if (req.getRequestHeaderFields().getField("Transfer-Encoding") == "chunked")
				{
					processingChunkedRequest = true;
				}
				postRequest(req, res, serverConfig, path);
			}
			// std::cout << "Processing chunked request" << std::endl;
			// std::cout << req.getMessageBody().toString() << std::endl;
			// ChunkedRequestReader reader("var/www/upload/files/testfile", clientSocket);
			// reader.processRequest();
            // postRequest(req, res, serverConfig, path);
        }
        else if (method == "DELETE")
        {
            deleteRequest(req, res, serverConfig, path);
        }
    }
    catch (const HTTPException& e)
    {
        throw e;
    }
}
void RequestHandler::getRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig, const std::string& path)
{
    (void)serverConfig;
    std::ifstream file(path);
    if (file.is_open() == false)
    {
        throw HTTPException(FORBIDDEN);
    }
    res.setStatusLine(req.getRequestLine().getHTTPVersion(), OK, "OK");
    std::string line;
    while (std::getline(file, line))
    {
        // TODO: 파일 마지막에 개행이 없는 경우 개행이 추가로 들어가는 문제
        res.addMessageBody(line + "\n");
    }
    file.close();
    addSemanticHeaderFields(res);
    addContentType(res, path);
}
void RequestHandler::headRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig, const std::string& path)
{
    getRequest(req, res, serverConfig, path);
    res.clearMessageBody();
}
void RequestHandler::postRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig, const std::string& path)
{
    (void)req;
    (void)res;
    (void)serverConfig;
    (void)path;
    res.setStatusLine(req.getRequestLine().getHTTPVersion(), OK, "OK");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><body><h1>POST Request</h1></body></html>");
    addSemanticHeaderFields(res);
}
void RequestHandler::deleteRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig, const std::string& path)
{
    (void)req;
    (void)res;
    (void)serverConfig;
    (void)path;
    res.setStatusLine(req.getRequestLine().getHTTPVersion(), OK, "OK");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><body><h1>DELETE Request</h1></body></html>");
    addSemanticHeaderFields(res);
}
void RequestHandler::addSemanticHeaderFields(ResponseMessage& res)
{
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[128];
    // Date: Tue, 15 Nov 1994 08:12:31 GMT
    // NOTE: strftime() 함수는 c함수라서 평가에 어긋남, 하지만 Date 헤더 필드가 필수가 아니라서 보너스 느낌으로 넣어둠
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    std::string date = buffer;

    res.addResponseHeaderField("Content-Length", res.getMessageBodySize());
    res.addResponseHeaderField("Server", "webserv");
    res.addResponseHeaderField("Date", date);
}
void RequestHandler::addContentType(ResponseMessage& res, const std::string& path)
{
    // NOTE: Config에 MIME 타입 추가
    std::string ext = path.substr(path.find_last_of('.') + 1);
    if (ext == "html")
    {
        res.addResponseHeaderField("Content-Type", "text/html");
    }
    else if (ext == "css")
    {
        res.addResponseHeaderField("Content-Type", "text/css");
    }
    else if (ext == "js")
    {
        res.addResponseHeaderField("Content-Type", "text/javascript");
    }
    else if (ext == "jpg" || ext == "jpeg")
    {
        res.addResponseHeaderField("Content-Type", "image/jpeg");
    }
    else if (ext == "png")
    {
        res.addResponseHeaderField("Content-Type", "image/png");
    }
    else
    {
        res.addResponseHeaderField("Content-Type", "application/octet-stream");
    }
}
void RequestHandler::handleException(const HTTPException& e, ResponseMessage& res)
{
    if (e.getStatusCode() == BAD_REQUEST)
    {
        badRequest(res);
    }
    else if (e.getStatusCode() == NOT_FOUND)
    {
        notFound(res);
    }
    else if (e.getStatusCode() == METHOD_NOT_ALLOWED)
    {
        methodNotAllowed(res);
    }
    else if (e.getStatusCode() == HTTP_VERSION_NOT_SUPPORTED)
    {
        httpVersionNotSupported(res);
    }
    else if (e.getStatusCode() == URI_TOO_LONG)
    {
        uriTooLong(res);
    }
    else if (e.getStatusCode() == FORBIDDEN)
    {
        forbidden(res);
    }
}
void RequestHandler::badRequest(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", BAD_REQUEST, "Bad Request");
    res.addMessageBody("<html><head><title>400 Bad Request</title></head><body><h1>400 Bad Request</h1></body></html>");
    res.addResponseHeaderField("Content-Type", "text/html");
    addSemanticHeaderFields(res);
}
void RequestHandler::notFound(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", NOT_FOUND, "Not Found");
    res.addMessageBody("<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>");
    res.addResponseHeaderField("Content-Type", "text/html");
    addSemanticHeaderFields(res);
}
void RequestHandler::methodNotAllowed(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", METHOD_NOT_ALLOWED, "Method Not Allowed");
    res.addMessageBody("<html><head><title>405 Method Not Allowed</title></head><body><h1>405 Method Not Allowed</h1></body></html>");
    res.addResponseHeaderField("Content-Type", "text/html");
    addSemanticHeaderFields(res);
}
void RequestHandler::httpVersionNotSupported(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported");
    res.addMessageBody("<html><head><title>505 HTTP Version Not Supported</title></head><body><h1>505 HTTP Version Not Supported</h1></body></html>");
    res.addResponseHeaderField("Content-Type", "text/html");
    addSemanticHeaderFields(res);
}
void RequestHandler::uriTooLong(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", URI_TOO_LONG, "Request-URI Too Long");
    res.addMessageBody("<html><head><title>414 Request-URI Too Long</title></head><body><h1>414 Request-URI Too Long</h1></body></html>");
    res.addResponseHeaderField("Content-Type", "text/html");
    addSemanticHeaderFields(res);
}
void RequestHandler::forbidden(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", FORBIDDEN, "Forbidden");
    res.addMessageBody("<html><head><title>403 Forbidden</title></head><body><h1>403 Forbidden</h1></body></html>");
    res.addResponseHeaderField("Content-Type", "text/html");
    addSemanticHeaderFields(res);
}
