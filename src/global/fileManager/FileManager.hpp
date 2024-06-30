#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <sys/stat.h>
#include <unistd.h>
#include <string>

class FileManager
{
public:
    enum eStat
    {
        NONE,
        DIRECTORY,
        FILE,
        READABLE,
        WRITABLE
    };
    static void setNonBlocking(int fd);
    static int getFileStatus(const std::string& path);
    static bool isReadable(const std::string& path);
    static bool isWritable(const std::string& path);
    static const std::string listDirectoryContents(const std::string& path);
};

#endif
