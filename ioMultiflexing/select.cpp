#include <sys/time.h>
#include <unistd.h>
#include <iostream>

#define TIMEOUT 5
#define BUF_LEN 1024

int main(void)
{
    struct timeval tv;
    fd_set readfds;
    int ret;

    /* 표준 입력에서 입력을 기다리기 위한 준비 */
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    /* select가 5초 동안 기다리도록 timeval 구조체 설정 */
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;

    ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
    if (ret == -1)
    {
        perror("select");
        return 1;
    }
    else if (!ret)
    {
        std::cout << TIMEOUT << " seconds elapsed.\n";
        return 0;
    }

    /* select가 양수를 반환 */
    if (FD_ISSET(STDIN_FILENO, &readfds))
    {
        char buf[BUF_LEN + 1];
        int len;

        /* read는 블록되지 않음 */
        if (len == -1)
        {
            perror("read");
            return 1;
        }

        if (len)
        {
            buf[len] = '\0';
            std::cout << "read: " << buf << '\n';
        }

        return 0;
    }

    std::cerr << "This should not happen!\n";
    return 1;
}
