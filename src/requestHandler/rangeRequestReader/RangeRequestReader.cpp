#include "RangeRequestReader.hpp"
#include "RequestHandler.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>

RangeRequestReader::RangeRequestReader(const std::string &filePath) : filePath(filePath) {}

RangeRequestReader::~RangeRequestReader() {}

void RangeRequestReader::addRange(size_t start, size_t end)
{
    ranges.push_back(std::make_pair(start, end));
}

std::string RangeRequestReader::readRange(size_t start, size_t end)
{
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd == -1)
    {
        throw std::runtime_error("Unable to open file");
    }

    if (lseek(fd, start, SEEK_SET) == -1)
    {
        close(fd);
        throw std::runtime_error("Error seeking file");
    }

    size_t length = end - start + 1;
    std::vector<char> buffer(length);
    ssize_t bytesRead = read(fd, buffer.data(), length);
    if (bytesRead == -1)
    {
        close(fd);
        throw std::runtime_error("Error reading file");
    }

    close(fd);
    return std::string(buffer.begin(), buffer.begin() + bytesRead);
}

std::string RangeRequestReader::processRequest()
{
    std::stringstream responseBody;
    std::string boundary = "BOUNDARY_STRING";

    for (size_t i = 0; i < ranges.size(); ++i)
    {
        size_t start = ranges[i].first;
        size_t end = ranges[i].second;
        std::string content = readRange(start, end);

        responseBody << "--" << boundary << "\r\n";
        responseBody << "Content-Type: application/octet-stream\r\n";
        responseBody << "Content-Range: bytes " << start << "-" << end << "/*\r\n\r\n";
        responseBody << content << "\r\n";
    }

    responseBody << "--" << boundary << "--\r\n";

    return responseBody.str();
}
