#include "FileManager.hpp"
#include <dirent.h>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "SysException.hpp"

// 소켓 논블로킹 설정
void FileManager::setNonBlocking(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
    {
        close(fd);
        throw SysException(FAILED_TO_SET_NON_BLOCKING);
    }
}
int FileManager::getFileStatus(const std::string& path)
{
    struct stat info;
    // 파일이 없거나 접근 권한이 없는 경우
    if (stat(path.c_str(), &info) != 0)
    {
        return NONE;
    }
    // 디렉토리인지 확인
    if (info.st_mode & S_IFDIR)
    {
        return DIRECTORY;
    }
    // 일반 파일인지 확인
    else if (info.st_mode & S_IFREG)
    {
        return FILE;
    }
    else
    {
        return NONE;
    }
}
bool FileManager::isReadable(const std::string& path)
{
    return (access(path.c_str(), R_OK) == 0);
}
bool FileManager::isWritable(const std::string& path)
{
    return (access(path.c_str(), W_OK) == 0);
}
const std::string FileManager::listDirectoryContents(const std::string& path)
{
    struct dirent* entry;
    struct stat statbuf;

    DIR* dir = opendir(path.c_str());
    std::ostringstream oss;

    oss << "<html>"
        << "<head><title>Index of " << path << "</title></head>"
        << "<body>"
        << "<h1>Index of " << path << "</h1>"
        << "<hr>"
        << "<pre>"
        << "<a href=\"../\">../</a>\n";
    if (dir == NULL)
    {
        std::cerr << "Failed to open directory" << std::endl;
        return "";
    }

    while ((entry = readdir(dir)) != NULL)
    {
        std::ostringstream line;
        // 경로와 파일명을 결합하여 전체 경로 생성
        std::string fullPath = std::string(path) + "/" + entry->d_name;

        // 파일 상태 정보 얻기
        if (stat(fullPath.c_str(), &statbuf) == -1)
        {
            std::cerr << "Failed to get file status for " << entry->d_name << std::endl;
            continue;
        }
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        size_t width = 51;  // 50글자보다 길 경우에 substr(0, 47) + "..>" 하여 50글자로 제한
        std::string dirName(entry->d_name);
        if (dirName.size() > 50)
        {
            line << "<a href=\"" << dirName << (S_ISDIR(statbuf.st_mode) ? "/" : "") << "\">" << dirName.substr(0, 47) << "..>" << "</a>";
            width = 1;
        }
        else
        {
            line << "<a href=\"" << dirName << (S_ISDIR(statbuf.st_mode) ? "/" : "") << "\">" << dirName << (S_ISDIR(statbuf.st_mode) ? "/" : "") << "</a>";
            width -= dirName.size() + (S_ISDIR(statbuf.st_mode) ? 1 : 0);
        }

        std::tm* tm = std::localtime(&statbuf.st_mtime);
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%d-%b-%Y %H:%M", tm);

        std::string spaces(width, ' ');
        oss << line.str() << spaces << buffer;
        oss << std::right << std::setw(20) << (S_ISDIR(statbuf.st_mode) ? "-" : std::to_string(statbuf.st_size)) << '\n';
    }
    oss << "</pre><hr></body></html>";
    closedir(dir);
    return oss.str();
}
