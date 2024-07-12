#!/bin/bash

CR="\r"

# 테스트할 요청과 기대하는 응답 코드 배열
requests=(
"GET / HTTP/1.1$CR
Host: localhost$CR
Connection: keep-alive$CR
Content-Length: 10000000$CR
Number: 1$CR
Connection: close$CR
$CR
dsadsadsa" 413

"GET / HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Content-Length: 0$CR
Connection: close$CR
Number: 2$CR
$CR
" 200

"GET /error HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Number: 3$CR
$CR
" 404

"POST / HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Number: 4$CR
$CR
" 200

"GET / HTTP/1.1$CR
Host: localhost:8080$CR
Connection: close$CR
Number: 5$CR
$CR
" 200

"GET / HTTP/1.1$CR
Number: 6$CR
$CR
" 400

"GET / HTTP/1.1$CR
Host: seunan:8081$CR
Connection: close$CR
Number: 7$CR
$CR
" 200

"GET /permission_denied/forbidden HTTP/1.1$CR
Host: localhost:8080$CR
Connection: close$CR
Number: 8$CR
$CR
" 403

"GET / HTTP/1.1$CR
Host: localhost:8080$CR
Connection: close$CR
Number: 9$CR
$CR
" 200

"GET / HTTP/1.1$CR
Host: localhost:8080$CR
Connection: close$CR
Number: 10$CR
$CR
" 200

"GET / HTTP/1.1$CR
Host: localhost:8080$CR
Connection: close$CR
Date: Sun Nov 6 08:49:37 1994$CR
Number: 11$CR
$CR
" 200

"GET / HTTP/1.1       a$CR
Host: localhost:8080$CR
Connection: close$CR
Number: 12$CR
$CR
" 400

"GET / HTTP/1.1$CR
Host: localhost:8080$CR
Connection: close$CR
Number: 13$CR
$CR
Connection: close$CR
Content-Type: text/html$CR
$CR
bodybody" 200

"GET / HTTP/1.1$CR
Host: localhost:8080$CR
Connection: close$CR
Content-Type: text/html$CR
Number: 14$CR
$CR
bodybody" 200

"GET /uritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolonguritoolong HTTP/1.1$CR
Host: localhost:8080$CR
Connection: close$CR
Content-Type: text/html$CR
Number: 15$CR
$CR
" 414

"DELETE /notfound.html HTTP/1.1$CR
Host: localhost:8080$CR
Connection: close$CR
Number: 16$CR
$CR
" 404

"DELETE /delete HTTP/1.1$CR
Host: localhost:8080$CR
Connection: close$CR
Number: 17$CR
$CR
" 200

"DELETE /permission_denied/not_allow HTTP/1.1$CR
Host: localhost:8080$CR
Connection: close$CR
Number: 18$CR
$CR
" 403

"GET / HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Content-Length: 100$CR
Number: 19$CR
Connection: close$CR
$CR
dsadsadsadsadsadsadsadsaddsadsadsadsadsadsadsadsaddsadsadsadsadsadsadsadsaddsadsadsadsadsadsadsadsad" 200

"GET / HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Content-Length: 101$CR
Number: 20$CR
Connection: close$CR
$CR
dsadsadsadsadsadsadsadsaddsadsadsadsadsadsadsadsaddsadsadsadsadsadsadsadsaddsadsadsadsadsadsadsadsad" 413

"GET / HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Content-Length: 100$CR
Number: 21$CR
Connection: close$CR
$CR
dsadsadsadsadsadsadsadsaddsadsadsadsadsadsadsadsaddsadsadsadsadsadsadsadsaddsadsadsadsadsadsadsadsada" 200

"PUT / HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Content-Length: 100$CR
Number: 22$CR
$CR
" 405

"GET / HTTP/1.2$CR
Host: localhost$CR
Connection: close$CR
Number: 23$CR
$CR
" 505

"GET /index.py HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Number: 24$CR
$CR
" 200

"GET /redirect HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Number: 25$CR
$CR
" 302

"POST /upload HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Content-Length: 100$CR
Number: 26$CR
Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"$CR
Content-Type: text/plain$CR
$CR
dsadsadsadsadsadsadsadsaddsadsadsadsadsadsadsadsaddsadsadsadsadsadsadsadsaddsadsadsadsadsadsadsadsad" 201

"POST /upload/index.html HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Content-Length: 101$CR
Number: 27$CR
Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"$CR
Content-Type: text/html$CR
$CR
<html><body><h1>It works!</h1><p>lorem ipsumlorem ipsumlorem ipsumlorem ipsum dasad</p></body></html>" 413

"POST /upload/index.html HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Content-Length: 100$CR
Number: 28$CR
Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"$CR
Content-Type: text/html$CR
$CR
<html><body><h1>It works!</h1><p>lorem ipsumlorem ipsumlorem ipsumlorem ipsumipsumlorem ipsumdad</p>" 201

"POST /upload/index.html HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Content-Length: 14$CR
Number: 29$CR
Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"$CR
Content-Type: text/html$CR
$CR
</body></html>" 200

"DELETE /upload/test.txt HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Number: 30$CR
$CR
" 200

"DELETE /upload/test.txt HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Number: 31$CR
$CR
" 404

"DELETE /upload/index.html HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Number: 32$CR
$CR
" 200

"POST /upload/test.txt HTTP/1.1$CR
Host: localhost$CR
Content-Type: text/plain$CR
Transfer-Encoding: chunked$CR
Number: 33$CR
Connection: close$CR
$CR
8$CR
Mozilla $CR
11$CR
Developer Network$CR
0$CR
$CR
" 201

"GET /proxy HTTP/1.1$CR
Host: localhost$CR
Connection: close$CR
Number: 34$CR
$CR
" 200
)

# ---------------------------------------------------------------------------------------------------

# 웹서버 kqueue, Conpig 설정 시간 (초)
WEBSERVER_CREATE_TIME=1

# 서버 응답 대기 시간 (초) - CGI 스크립트 처리 시간을 고려하여 적절히 설정
RESPONSE_WAIT_TIME=0.2

# netcat 명령어 타임아웃 시간 (초)
NC_TIMEOUT=5

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

setup_test_environment() {
    # Kill any running instances of webserv
    killall -9 webserv > /dev/null 2>&1
    
    # Create necessary files and directories
    mkdir -p ../../www/permission_denied log
    touch ../../www/permission_denied/not_allow ../../www/permission_denied/forbidden ../../www/delete
    rm -f log/*
    chmod 000  ../../www/permission_denied/forbidden
    chmod 444 ../../www/permission_denied/not_allow
    chmod 544 ../../www/permission_denied/
}

start_webserv() {
    clear && echo -e "${WHITE}webserv tester$NC"
    echo -e "${BLUE}webserver building...$NC"
    make -C ../../ mem -j > /dev/null 2>&1
    echo -e "${BLUE}webserver running...$NC"

    python3.10 app.py > /dev/null 2>&1 & APP_PID=$!
    ../../webserv test.conf & WEBSERV_PID=$!
    sleep $WEBSERVER_CREATE_TIME
}

perform_tests() {
    # Create a file to store unexpected responses
    echo -n > unexpected_response.txt
    
    # Iterate over the requests and expected responses
    for ((i=0; i<${#requests[@]}; i+=2)); do
        request="${requests[$i]}"
        expected_status_code="${requests[$((i+1))]}"
        
        # Send the request and get the response
        response=$( (echo -ne "${request}"; sleep $RESPONSE_WAIT_TIME) | nc -w$NC_TIMEOUT localhost 8080)
        status_code=$(echo "${response}" | awk 'NR==1 {print $2}')
        
        # Check if the response status code matches the expected status code
        if [ "$status_code" -eq "$expected_status_code" ]; then
            echo -e "${GREEN}TEST $((i/2 + 1)): PASSED${NC}"
        else
            echo -e "${RED}TEST $((i/2 + 1)): FAILED${NC}"
            echo -e "Expected: ${expected_status_code}, Got: ${status_code}\nRequest:\n${request}\nResponse:\n${response}\n" >> unexpected_response.txt
        fi
    done
}

clean_up() {
    chmod -R 777 ../../www/permission_denied
    rm -rf ../../www/permission_denied
    rm -rf ../../www/upload/test.txt

    # webserv 프로그램 종료
    kill -9 $WEBSERV_PID > /dev/null 2>&1
    kill -9 $APP_PID > /dev/null 2>&1
}

setup_test_environment
start_webserv
perform_tests
clean_up
