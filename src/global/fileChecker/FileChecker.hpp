#ifndef FILE_CHECKER_HPP
#define FILE_CHECKER_HPP

#include <sys/stat.h>
#include <unistd.h>
#include <string>

class FileChecker
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
    static eStat getFileStatus(const std::string& path);
    static bool isReadable(const std::string& path);
    static bool isWritable(const std::string& path);
};

#endif
