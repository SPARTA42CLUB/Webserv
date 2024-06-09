# poll

## 운영체제에서의 polling이란?
polling은 컴퓨터 또는 제어 장치가 외부 장치가 준비 상태 또는 상태를 확인할 때까지 대기하는 프로세스로, 종종 낮은 수준의 하드웨어를 사용합니다.\
예를 들어, 프린터가 병렬 포트를 통해 연결된 경우 컴퓨터는 프린터가 다음 문자를 수신할 때까지 대기합니다. 이것은 때때로 `busy-wait` polling과 동의어로 사용됩니다.\
이 경우 I/O 작업이 필요한 경우 컴퓨터는 I/O 장치가 준비될 때까지 I/O 장치의 상태를 확인하는 것 외에는 아무 작업도 수행하지 않습니다.\
즉, 컴퓨터는 장치가 준비될 때까지 기다립니다.\
polling은 또한 장치의 준비 상태를 반복적으로 확인하고 그렇지 않은 경우 컴퓨터가 다른 작업으로 돌아가는 상황을 나타냅니다.\
바쁜 대기만큼 CPU 사이클을 낭비하지는 않지만 일반적으로 polling, 인터럽트 구동 I/O의 대안만큼 효율적이지는 않습니다

- [폴링(Polling), 인터럽트(Interrupt), DMA(Direct Memory Access)](https://jaebworld.tistory.com/entry/%ED%8F%B4%EB%A7%81Polling-%EC%9D%B8%ED%84%B0%EB%9F%BD%ED%8A%B8Interrupt-DMADirect-Memory-Access)
- [Polling (computer science) | Wikipedia](https://en.wikipedia.org/wiki/Polling_(computer_science))

## I/O Multiplexing
I/O 멀티플렉싱은 [poll](https://en.wikipedia.org/wiki/Poll_(Unix)) 및 [select (Unix)](https://en.wikipedia.org/wiki/Select_(Unix))와 같은 시스템 호출을 사용하여 단일 이벤트 루프에서 여러 I/O Events를 처리하는 기술입니다.

- [I/O Multiplexing (select vs. poll vs. epoll/kqueue) - Problems and Algorithms](https://nima101.github.io/io_multiplexing)
- [I/O multiplexing: select(), poll(), kqueue() — rien](https://ri3n.tistory.com/94)
- [[OS] I/O 멀티플렉싱 (select, poll, epoll, kqueue)](https://12bme.tistory.com/693)
- [I/O Multiplexing(select, .. : 네이버블로그](https://blog.naver.com/tius1234/220229189161)

### poll()
```c
int poll(struct pollfd fds[], nfds_t nfds, int timeout);
```
- [poll](https://pubs.opengroup.org/onlinepubs/9699919799/functions /poll.html)
- [poll (Unix)](https://en.wikipedia.org/wiki/Poll_(Unix))

### epoll()
```c
int epoll_create1(int flags);
```
[epoll](https://en.wikipedia.org/wiki/Epoll)

### kqueue()
```c
int kqueue(void);
int kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout);
```
[kqueue](https://en.wikipedia.org/wiki/Kqueue)
<details>
<summary><b>man kqueue</b></summary>

</details>

### select()
```c
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout);
```
[select (Unix)](https://en.wikipedia.org/wiki/Select_(Unix))
