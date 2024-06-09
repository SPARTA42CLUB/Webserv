#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
using namespace std;

#define TIMEOUT 5

int main (void)
{
    // pollfd 구조체의 배열을 선언합니다. 이 구조체는 파일 디스크립터, 관심 있는 이벤트, 발생한 이벤트를 추적합니다.
	struct pollfd fds[2];

    /* watch stdin for input */
    // fds[0]에 대해 읽기 이벤트를 관찰하도록 설정합니다.
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    /* watch stdout for ability to write */
    // fds[0]에 대해 쓰기 이벤트를 관찰하도록 설정합니다.
    fds[1].fd = STDOUT_FILENO;
    fds[1].events = POLLOUT;

    // fds 배열에 있는 파일 디스크립터가 읽기 또는 쓰기를 위해 준비되었는지 확인합니다. TIMEOUT * 1000은 대기 시간을 밀리초 단위로 설정
    int ret = poll(fds, 2, TIMEOUT * 10000);

    if (ret == -1) {
        perror ("poll");
        return 1;
    }

    if (!ret) 
    {
        std::cout << TIMEOUT << " seconds elapsed.\n";
        return 0;
    }

    if (fds[0].revents & POLLIN)
    {
        std::cout << "stdin is readable\n";
    }

    if (fds[1].revents & POLLOUT)
    {
        std::cout << "stdout is writable\n";
    }

	return 0;

}
