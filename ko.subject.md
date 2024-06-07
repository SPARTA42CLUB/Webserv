# Webserv

HTTP 서버를 작성하는 과제

## Introduction

HTTP(Hypertext Transfer Protocol)는 WWW(World Wide Web)의 데이터 통신을 위한 기반이며, 하이퍼텍스트 문서에는 사용자가 쉽게 액세스할 수 있는 다른 리소스에 대한 하이퍼링크가 포함됩니다.

Hypertext 문서에는 사용자가 쉽게 액세스할 수 있는 다른 리소스에 대한 하이퍼링크가 포함됩니다.
예를 들어, 마우스 클릭이나 웹 브라우저에서 화면을 탭하는 등의 방법으로 다른 리소스에 액세스할 수 있습니다.
HTTP는 Hypertext와 (World Wide Web)을 용이하게 하기 위해 개발되었습니다.

웹 서버의 주요 기능은 웹 페이지를 저장, 처리, 클라이언트에게 전달하는 것입니다.
클라이언트와 서버 간의 통신은 HTTP을 사용하여 이루어집니다.

전달되는 페이지는 대부분 HTML 문서이며 이미지를 포함할 수 있습니다,
스타일시트, 스크립트 등이 포함될 수 있습니다.
트래픽이 많은 웹사이트에는 여러 웹 서버를 사용할 수 있습니다.
사용자 에이전트(일반적으로 웹 브라우저 또는 웹 크롤러)는 다음과 같은 방법으로 통신을 시작합니다.
HTTP를 사용하여 특정 리소스를 요청하면 서버는 해당 리소스의 콘텐츠 또는 오류 메시지로 응답합니다.
리소스 또는 리소스를 제공할 수 없는 경우 오류 메시지로 응답합니다. 리소스는 일반적으로 서버의 보조 저장소에 있는 실제 파일이지만 반드시 그런 것은 아니며 웹서버가 어떻게 구현되었는지에 따라 웹서버가 구현되는 방식에 따라 다릅니다.

웹서버의 주요 기능은 콘텐츠를 제공하는 것이지만, HTTP의 완전한 구현에는 다음과 같은 기능도 포함됩니다.
클라이언트로부터 콘텐츠를 수신하는 방법도 포함됩니다. 이 기능은 웹 양식 제출에 사용됩니다.
양식을 제출하는 데 사용됩니다.

## General rules

- 프로그램은 어떤 상황에서도 충돌해서는 안 되며(메모리가 부족하더라도) 예기치 않게 종료되어서는 안 됩니다.
- 소스 파일을 컴파일하는 메이크파일을 제출해야 하며 relink가 되어서는 안 됩니다.
- 메이크파일에는 최소한 다음 규칙이 포함되어야 합니다: \
    $(NAME), all, clean, fclean, re.
- 코드를 c++로 컴파일하고 `-Wall -Wextra -Werror` 플래그를 사용하세요.
- 코드는 C++ 98 표준을 준수해야 합니다.
- 항상 가능한 한 많은 C++ 기능을 사용하여 개발하세요. (예를 들어 <string.h>보다 <cstring> 선택).
-외부 라이브러리 및 부스트 라이브러리 사용은 금지됩니다.

## Mandatory part

|Program name|webserv|
|---|---|
|Turn in files|Makefile, *.{h, hpp}, *.{cpp}, *.tpp, *.ipp, configuration files|
|Makefile|NAME, all, clean, fclean, re|
|Arguments|[A configuration file]|
|External functs|Everything in C++ 98. execve, dup, dup2, pipe, strerror, gai_strerror, errno, dup, dup2, fork, socketpair, htons, htonl, ntohs, ntohl, select, poll, epoll (epoll_create, epoll_ctl, epoll_wait), kqueue (kqueue, kevent), socket, accept, listen, send, recv, chdir bind, connect, getaddrinfo, freeaddrinfo, setsockopt, getsockname, getprotobyname, fcntl, close, read, write, waitpid, kill, signal, access, stat, open, opendir, readdir and closedir|
|Libft authorized|not applicable|
|Description|A HTTP server in C++ 98|

`./webserv [configuration file]`

> 과제와 평가 척도에 `poll()`이 언급되어 있더라도, `select()`, `kqueue()` 또는 `epoll()`과 같은 동등한 함수를 사용할 수 있습니다.

> 프로젝트를 시작하기 전에 RFC를 읽고 텔넷과 NGINX로 몇 가지 테스트를 해보세요.\
> 모든 RFC를 구현할 필요는 없더라도 필요한 기능을 개발하는 데 도움이 될 것입니다.

### Requirements

- 프로그램에서 configuration file을 인수로 받거나 기본 경로를 사용해야 합니다.
- 다른 웹 서버를 실행시키면 안 됩니다.
- 서버가 block이 되지 않아야 하며 필요한 경우 클라이언트가 제대로 바운스될 수 있어야 합니다.
- 클라이언트와 서버 간의 모든 I/O에 대해 반드시 **non-blocking**이 되어야 하며 1개의 `poll()`(또는 이에 상응하는 함수)만 사용해야 합니다.
- `poll()`(또는 이와 동등한 함수)은 읽기와 쓰기를 동시에 확인해야 합니다.
- 읽기 또는 쓰기 작업 후 errno 값을 확인하는 것은 엄격히 금지됩니다.
- 설정 파일을 읽기 전에 poll()(또는 이와 동등한 함수)를 사용할 필요가 없습니다.
- 모든 매크로를 사용할 수 있으며 `FD_SET`, `FD_CLR`, `FD_ISSET`, `FD_ZERO`처럼 정의할 수 있습니다(매크로가 무엇을 어떻게 수행하는지 이해하는 것이 매우 유용합니다).
- 서버에 대한 요청이 영원히 중단되지 않아야 합니다.
- 서버가 선택한 웹 브라우저와 호환되어야 합니다.
- NGINX는 HTTP 1.1을 준수하며 헤더와 응답 동작을 비교하는 데 사용될 수 있습니다.
- HTTP 응답 상태 코드가 정확해야 합니다.
- 기본 오류 페이지가 제공되지 않는 경우 서버에 기본 오류 페이지가 있어야 합니다.
- `fork`를 CGI가 아닌 다른 것(예: PHP, Python 등)에 사용할 수 없습니다.
- 완전히 정적인 웹사이트를 제공할 수 있어야 합니다.
- 클라이언트가 파일을 업로드할 수 있어야 합니다.
- 최소 `GET`, `POST`, `DELETE` 메서드가 필요합니다.
- 서버에 대한 스트레스 테스트를 수행합니다. 서버는 항상 가용성을 유지해야 합니다.
- 서버가 여러 포트를 수신할 수 있어야 합니다(구성 파일 참조).

> 반드시 non-blocking file descriptor를 사용해야 하기 때문에,  `poll()` 없이 `read/recv` 또는 `write/send`를 사용할 수 있으며 서버는 block되지 않습니다.\
> 하지만 시스템 리소스를 더 많이 소비하게 됩니다.\
> 따라서 `poll()`(또는 이와 동등한 함수)를 사용하지 않고 파일 디스크립터에서 `read/recv`  또는 `write/send`을 시도하면 등급이 0이 됩니다.

#### For MacOS only

> MacOS는 다른 유닉스 OS와 같은 방식으로 `write()`를 구현하지 않으므로 `fcntl()`을 사용할 수 있습니다.\
> 다른 유닉스 OS와 유사한 동작을 얻으려면 non-blocking 모드에서 file descriptor을 사용해야 합니다.

> 그러나 다음과 같은 경우에만 fcntl()을 사용할 수 있습니다.\
> flag:\
> `F_SETFL`, `O_NONBLOCK` 및 `FD_CLOEXEC`. \
> 다른 플래그는 금지됩니다.

#### Configuration file
> NGINX의 `server` 부분에서 영감을 얻을 수 있습니다.

구성 파일에서 다음을 수행할 수 있어야 합니다:
- 각 `server`의 포트와 호스트를 선택합니다.
- `server_names`을 설정할지 여부를 설정합니다.
- `host:port`의 첫 번째 서버가 이 `host:port`의 기본값이 됩니다. (다른 서버에 속하지 않는 모든 요청에 응답한다는 의미).
- 기본 오류 페이지를 설정합니다.
- 클라이언트 본문 크기 제한.
- 다음 규칙/구성 중 하나 또는 여러 개를 사용하여 경로를 설정합니다. (경로는 정규식을 사용하지 않습니다.
    - 경로에 허용되는 HTTP 메소드 목록을 정의합니다.
    - HTTP 리다이렉션을 정의합니다.
    - 파일을 검색할 디렉토리 또는 파일을 정의합니다.(예, URL `/kapouet`이 `/tmp/www`로 라우팅된 경우, URL `/kapouet/pouic/toto/pouet`가 `/tmp/www/pouic/toto/pouet`로 라우팅되어야 합니다).
    - 디렉토리 목록을 켜거나 끕니다.
    - 요청이 디렉토리인 경우 응답할 기본 파일을 설정합니다.
    - 특정 파일 확장자(예: .php)를 기준으로 CGI를 실행합니다. 
    - POST 및 GET 메소드와 함께 작동하도록 설정합니다.
    - 경로가 업로드된 파일을 수락하고 저장할 위치를 설정합니다.
        - CGI가 뭔지 궁금하신가요?
        - CGI를 직접 호출하지 않으므로 전체 경로를 `PATH_INFO`로 사용합니다.
        - 청크된 요청의 경우 서버가 청크를 해제해야 한다는 점만 기억하세요. 하지 않으면 CGI는 본문의 끝을 EOF로 예상합니다.
        - CGI의 출력도 마찬가지입니다. CGI에서 content_length가 반환되지 않으면 EOF는 반환된 데이터의 끝을 표시합니다.
        - 프로그램은 요청된 파일을 첫 번째 인자로 사용하여 CGI를 호출해야 합니다.
        - CGI는 상대 경로 파일 액세스를 위해 올바른 디렉토리에서 실행되어야 합니다.
        - 서버는 하나의 CGI(php-CGI, Python 등)로 작동해야 합니다.

평가 중에 모든 기능이 작동하는지 테스트하고 시연하려면 몇 가지 구성 파일과 기본 기본 파일을 제공해야 합니다.

> 한 동작에 대한 질문이 있는 경우 프로그램 동작과 NGINX의 동작을 비교해야 합니다.\
> 예를 들어 `server_name`이 어떻게 작동하는지 확인하세요.\
> 작은 테스터를 공유해 드렸습니다. 브라우저와 테스트에서 모든 것이 정상적으로 작동한다면 반드시 통과해야 하는 것은 아니지만 몇 가지 버그를 찾는 데 도움이 될 수 있습니다.

> 중요한 것은 복원력입니다. 서버는 절대 죽지 않아야 합니다.

> 하나의 프로그램으로만 테스트하지 마세요. Python이나 Golang 등 보다 편리한 언어로 테스트를 작성하세요. 원한다면 C나 C++로도 가능합니다.

## Bonus part
추가할 수 있는 추가 기능은 다음과 같습니다:
- 쿠키 및 세션 관리 지원 (간단한 예제 준비)
- 여러 CGI 처리

> 보너스 부분은 필수 부분이 완벽한 경우에만 평가됩니다. 완벽하다는 것은 필수 부분을 완벽하게 수행했으며 오작동 없이 작동한다는 의미입니다. 모든 필수 요건을 통과하지 못한 경우 보너스 부분은 전혀 평가되지 않습니다.
