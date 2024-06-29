#include "fileController.hpp"

#include <fcntl.h>
#include <unistd.h>

#include "SysException.hpp"

// 소켓 논블로킹 설정
void setNonBlocking(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
    {
        close(fd);
        throw SysException(FAILED_TO_SET_NON_BLOCKING);
    }
}
