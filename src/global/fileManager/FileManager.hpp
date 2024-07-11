#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include "LocationConfig.hpp"

namespace fileManager
{
    enum eStat
    {
        NONE,
        DIRECTORY,
        FILE,
        READABLE,
        WRITABLE
    };
    void setNonBlocking(int fd);
    int getFileStatus(const std::string& path);
    bool isExist(const std::string& path);
    bool isReadable(const std::string& path);
    bool isWritable(const std::string& path);
    bool isExecutable(const std::string& path);
    bool deleteFile(const std::string& path);
    const std::string listDirectoryContents(const std::string& path, const LocationConfig& locConfig);
}


#endif
