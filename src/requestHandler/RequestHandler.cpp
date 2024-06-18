#include "RequestHandler.hpp"
#include <unistd.h>
#include <fstream>

void RequestHandler::handleRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig)
{
    std::string reqTarget = req.getRequestLine().getRequestTarget();
    std::string method = req.getRequestLine().getMethod();
    std::string path = "";

    // NOTE: 요청한 URI가 locations에 없을 경우 확인
    std::map<std::string, LocationConfig>::const_iterator it = serverConfig.locations.find(reqTarget);
    if (it == serverConfig.locations.end())
    {
        for (std::map<std::string, LocationConfig>::const_iterator it = serverConfig.locations.begin(); it != serverConfig.locations.end(); it++)
        {
            if (it->first[it->first.size() - 1] == '/')
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

    // NOTE: allow_methods 블록이 없을 경우 모든 메소드 허용
    // WARNING: segment fault 발생
    /* 
    ==12581==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x000107b03567 at pc 0x0001043c3520 bp 0x00016ba486b0 sp 0x00016ba486a8
    READ of size 1 at 0x000107b03567 thread T0
    #0 0x1043c351c in std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>::__is_long[abi:ue170006]() const string:1734
    #1 0x1043ba13c in std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>::size[abi:ue170006]() const string:1168
    #2 0x1043c87a0 in bool std::__1::operator==[abi:ue170006]<std::__1::allocator<char>>(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&) string:3897
    #3 0x1043c7ae4 in std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const* std::__1::__find_impl[abi:ue170006]<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>, std::__1::__identity>(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&, std::__1::__identity&) find.h:34
    #4 0x1043baadc in std::__1::__wrap_iter<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*> std::__1::find[abi:ue170006]<std::__1::__wrap_iter<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>>(std::__1::__wrap_iter<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*>, std::__1::__wrap_iter<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const*>, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>> const&) find.h:81
    #5 0x1043b8ffc in RequestHandler::handleRequest(RequestMessage const&, ResponseMessage&, ServerConfig const&) RequestHandler.cpp:35
    #6 0x1043e2890 in Server::handleClientReadEvent(kevent&) Server.cpp:192
    #7 0x1043e03f8 in Server::run() Server.cpp:111
    #8 0x10440f528 in main main.cpp:18
    #9 0x18351a0dc  (<unknown module>)
     */
    // const LocationConfig& locConfig = it->second;
    // if (std::find(locConfig.allow_methods.begin(), locConfig.allow_methods.end(), method) == locConfig.allow_methods.end())
    // {
    //     throw HTTPException(METHOD_NOT_ALLOWED);
    // }

    // Method에 따른 처리
    try
    {
        if (req.getRequestLine().getMethod() == "GET")
        {
            getRequest(req, res, serverConfig, path);
        }
        else if (req.getRequestLine().getMethod() == "HEAD")
        {
            headRequest(req, res, serverConfig, path);
        }
        else if (req.getRequestLine().getMethod() == "POST")
        {
            postRequest(req, res, serverConfig, path);
        }
        else if (req.getRequestLine().getMethod() == "DELETE")
        {
            deleteRequest(req, res, serverConfig, path);
        }
    }
    catch (const HTTPException& e)
    {
        throw e;
    }
    
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
void RequestHandler::getRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig, const std::string& path)
{
    (void) serverConfig;
    std::ifstream file(path);
    if (file.is_open() == false)
    {
        throw HTTPException(FORBIDDEN);
    }
    res.setStatusLine(req.getRequestLine().getHTTPVersion(), std::to_string(OK), "OK");
    addContentType(res, path);
    std::string line;
    while (std::getline(file, line))
    {
        res.addMessageBody(line + "\n");
    }
    file.close();
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
    res.setStatusLine(req.getRequestLine().getHTTPVersion(), std::to_string(OK), "OK");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><body><h1>POST Request</h1></body></html>");
}
void RequestHandler::deleteRequest(const RequestMessage& req, ResponseMessage& res, const ServerConfig& serverConfig, const std::string& path)
{
    (void)req;
    (void)res;
    (void)serverConfig;
    (void)path;
    res.setStatusLine(req.getRequestLine().getHTTPVersion(), std::to_string(OK), "OK");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><body><h1>DELETE Request</h1></body></html>");
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
}
void RequestHandler::badRequest(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", std::to_string(BAD_REQUEST), "Bad Request");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><head><title>400 Bad Request</title></head><body><h1>400 Bad Request</h1></body></html>");
}
void RequestHandler::notFound(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", std::to_string(NOT_FOUND), "Not Found");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>");
}
void RequestHandler::methodNotAllowed(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", std::to_string(METHOD_NOT_ALLOWED), "Method Not Allowed");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><head><title>405 Method Not Allowed</title></head><body><h1>405 Method Not Allowed</h1></body></html>");
}
void RequestHandler::httpVersionNotSupported(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", std::to_string(HTTP_VERSION_NOT_SUPPORTED), "HTTP Version Not Supported");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><head><title>505 HTTP Version Not Supported</title></head><body><h1>505 HTTP Version Not Supported</h1></body></html>");
}
void RequestHandler::uriTooLong(ResponseMessage& res)
{
    res.setStatusLine("HTTP/1.1", std::to_string(URI_TOO_LONG), "Request-URI Too Long");
    res.addResponseHeaderField("Content-Type", "text/html");
    res.addMessageBody("<html><head><title>414 Request-URI Too Long</title></head><body><h1>414 Request-URI Too Long</h1></body></html>");
}
