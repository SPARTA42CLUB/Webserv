[C로 간단한 HTTP 웹서버 만들기 - bruinsslot.jp](https://bruinsslot.jp/post/simple-http-webserver-in-c/)

# A Simple Web Server in C++

## Socket
소켓이란 두 개 이상의 프로그램 간의 양방향 통신 링크의 끝점을 말한다.

- [Socket](https://en.wikipedia.org/wiki/Network_socket)
- [Socket in computer network]((https://www.geeksforgeeks.org/socket-in-computer-network/))
- [Socket Programming](https://www.ibm.com/docs/ko/i/7.3?topic=communications-socket-programming)

### Different of between Socket and Port
포트란 연결 끝점을 고유하게 식별하고 데이터를 특정 서비스로 전달하기 위해 할당되는 번호\
소켓은 `전송 프로토콜, IP 주소 및 포트 번호의 조합`이다.

- [Port](https://en.wikipedia.org/wiki/Port_(computer_networking))
- [Socket vs Port](https://www.geeksforgeeks.org/difference-between-socket-and-port/)

### Socket API
```c
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```
호출에 성공하면 반환되는 파일 기술자는 현재 프로세스에 열려 있지 않은 가장 낮은 번호의 FD를 반환하고 에러 시 -1을 반환한다.
- `domain`: 소켓이 사용할 프로토콜 체계(Protocol Family) 정보 
- `type`: 소켓의 데이터 전송방식에 대한 정보
- `protocol`: 더 컴퓨터간 통신에 사용되는 프로토콜 정보

#### Domain (Protocol Family)
| Name      | Protocol Family          |
| --------- | ------------------------ |
| AF_INET   | IPv4 인터넷 프로토콜 체계         |
| AF_INET6  | IPv6 인터넷 프로토콜 체계         |
| AF_LOCAL  | 로컬 통신을 위한 UNIX 프로토콜 체계   |
| AF_PACKET | Low Level 소켓을 위한 프로토콜 체계 |
| AF_IPX    | IPX 노벨 프로토콜 체계           |

실제 소켓이 사용할 최종 프로토콜 정보는 세 번째 인자이지만 이 프로토콜 체계 범위 내에 있다.

#### Type
프로토콜의 체계가 결정되었다고 해서 데이터 전송방식까지 결정된 것은 아니다.\
PF_INET에 해당하는 프로토콜 체계에도 둘 이상의 데이터 전송방식이 존재한다.\
따라서 타입은 **소켓의 데이터 전송방식을 의미한다.**

| Name      | Description |
| --------- | ----------- |
| SOCK_STREAM | 연결 지향형 소켓. TCP 프로토콜을 사용한다. |
| SOCK_DGRAM  | 비연결 지향형 소켓. UDP 프로토콜을 사용한다. |
| SOCK_RAW    | 소켓을 통해 IP 헤더를 직접 조작할 수 있는 소켓. |
| SOCK_SEQPACKET | 데이터의 경계가 존재하는 데이터 전송방식을 사용하는 소켓. |

#### Protocol
대부분의 경우 앞에서 본 2개의 인자의 정보로도 프로토콜 결정에 충분한 정보가 되어 이 인자를 0으로 넘겨주어도 원하는 데이터 소켓을 생성할 수 있다.

**하나의 프로토콜 체계 안에 데이터의 전송방식이 동일한 프로토콜이 둘 이상 존재하는 경우에 필요**

```c
// IPv4 인터넷 프로토콜 체계에서 동작하는 연결지향형 데이터 전송 소켓
int tcp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP );

// IPv4 인터넷 프로토콜 체계에서 동작하는 비연결지향형 데이터 전송 소켓
int udp_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP );
```
