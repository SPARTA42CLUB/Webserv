#!/bin/bash

# 테스트할 요청과 기대하는 응답 코드 배열
requests=(
"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" 200
"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" 200
"GET /error HTTP/1.1\r\nHost: localhost\r\n\r\n" 404
"POST / HTTP/1.1\r\nHost: localhost\r\n\r\n" 200
"GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n" 200
"GET / HTTP/1.1\r\n\r\n" 400
"GET / HTTP/1.1\r\nHost: seunan:8081\r\n\r\n" 200
"GET /forbidden.html HTTP/1.1\r\nHost: localhost:8080\r\n\r\n" 403
"GET / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: close\r\n\r\n" 200
"GET / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\n\r\n" 200
"GET / HTTP/1.1\r\nHost: localhost:8080\r\nDate: Sun Nov 6 08:49:37 1994\r\n\r\n" 200
"GET / HTTP/1.1       a\r\nHost: localhost:8080\r\nConnection: keep-alive\r\n\r\n" 400
"GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\nConnection: close\r\nContent-Type: text/html\r\n\r\nbodybody" 201
"GET / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: close\r\nContent-Type: text/html\r\n\r\nbodybody" 200
)

# ----------------------------------------------

# 색상 코드
BLACK='\033[90m'
RED='\033[91m'
GREEN='\033[92m'
YELLOW='\033[93m'
BLUE='\033[94m'
MAGENTA='\033[95m'
CYAN='\033[96m'
WHITE='\033[97m'
NC='\033[0m'

# 웹서버 kqueue, Conpig 설정 시간 (초)
WEBSERVER_CREATE_TIME=1

# 서버 응답 대기 시간 (초)
RESPONSE_WAIT_TIME=0.2

touch ../../www/forbidden.html && chmod 000 ../../www/forbidden.html

# webserv 프로그램을 백그라운드에서 실행
make -C ../../ mem && \
clear && echo -e "$WHITE webser tester$NC"

../../webserv test.conf & WEBSERV_PID=$!

# 응답을 저장할 파일을 생성 (기존 파일이 있으면 내용을 지우고 새로 생성)
echo -n > unexpected_response.txt

RESPONSE_WAIT_TIME=0.1

# 요청을 반복하면서 테스트
for ((i=0; i<${#requests[@]}; i+=2)); do
    request="${requests[$i]}"
    expected_status_code="${requests[$((i+1))]}"

    # 정수형 변수 예약
    declare -i status_code

    # 요청을 파일로 저장 (-e 옵션으로 이스케이프 문자 처리)
    # 네트워크 타이밍 문제:
    # nc 명령어는 매우 빠르게 실행되는 도구입니다. 따라서 echo 명령어가 요청을 보내자마자 nc가 연결을 닫아버릴 수 있습니다. sleep을 사용하면 요청을 보낸 후 잠시 동안 연결을 유지하게 되어 서버가 응답을 전송할 시간을 확보하게 됩니다.
    # 서버의 처리 시간:
    # CGI 스크립트가 실행되고 응답을 생성하는 데 시간이 걸릴 수 있습니다. sleep을 통해 연결을 잠시 동안 유지하면 서버가 응답을 생성하여 클라이언트에게 전송할 시간을 가질 수 있습니다. 요청을 보내고 바로 연결을 닫으면 서버가 응답을 전송하기 전에 연결이 끊어질 수 있습니다.
    # 버퍼링 문제:
    # 네트워크 통신에서는 데이터가 버퍼링될 수 있습니다. sleep을 통해 약간의 시간을 주면 데이터가 완전히 전송되고 처리될 수 있습니다. 즉, 클라이언트가 데이터를 전송하고 서버가 이를 완전히 수신 및 처리할 수 있는 시간을 확보하게 됩니다.
    # 프로세스 동기화:
    # nc와 같은 도구는 입력을 읽고 처리하는 동안 매우 짧은 시간 안에 종료될 수 있습니다. sleep을 사용하면 입력을 보낸 후 nc가 조금 더 오래 실행되어 서버의 응답을 기다리게 됩니다. 이는 클라이언트와 서버 간의 동기화 문제를 해결하는 데 도움이 됩니다.
    # 요약하자면, sleep을 사용하여 연결을 잠시 동안 유지하면 클라이언트가 요청을 보낸 후 서버의 응답을 받을 수 있는 충분한 시간을 확보하게 됩니다. 이는 네트워크 타이밍 문제, 서버의 처리 시간, 버퍼링 문제, 프로세스 동기화 문제 등을 해결하는 데 기여할 수 있습니다.
    response=$((echo -ne "${request}"; sleep $RESPONSE_WAIT_TIME) | nc -w 1 localhost 8081) # nc -w 1 옵션으로 1초 대기 후 응답 없으면 종료, nc 명령어의 출력을 없애기 위함
    status_code=$(echo "${response}" | awk 'NR==1 {print $2}') # Response의 Status Line에서 HTTP 상태 코드 추출

    # 요청을 보내고 응답 코드 확인
    if [ "$status_code" -eq "$expected_status_code" ]; then
        echo -e "$GREEN 테스트 $((i/2+1)): 성공 (HTTP 상태 코드: $status_code)$NC"
    else
        echo -e "$RED 테스트 $((i/2+1)): 실패 (기대한 HTTP 상태 코드: $expected_status_code, 실제 HTTP 상태 코드: $status_code)$NC"
        echo "Test $((i/2+1))" >> unexpected_response.txt
        echo "--------------------------------------REQUEST--------------------------------------" >> unexpected_response.txt
        echo -ne "${request}" >> unexpected_response.txt
        echo "--------------------------------------RESPONSE--------------------------------------" >> unexpected_response.txt
        echo -ne "${response}" >> unexpected_response.txt
    fi
done

rm -f ../../www/forbidden.html

# webserv 프로그램 종료
kill -9 $WEBSERV_PID > /dev/null 2>&1
