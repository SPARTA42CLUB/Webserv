#include "FileChecker.hpp"

FileChecker::eStat FileChecker::getFileStatus(const std::string& path)
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
bool FileChecker::isReadable(const std::string& path)
{
    return (access(path.c_str(), R_OK) == 0);
}
bool FileChecker::isWritable(const std::string& path)
{
    return (access(path.c_str(), W_OK) == 0);
}
