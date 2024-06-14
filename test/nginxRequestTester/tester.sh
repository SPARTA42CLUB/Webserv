#!/bin/bash

# 테스트할 요청과 기대하는 응답 코드 배열
requests=(
"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" 200
"GET / HTTP/1.1                 \r\nHost: localhost\r\n\r\n" 200
"GET /error HTTP/1.1\r\nHost: localhost\r\n\r\n" 404
"POST / HTTP/1.1\r\nHost: localhost\r\n\r\n" 405
"GET / HTTP/1.1\nHost: localhost:8080\n\n\n" 200
"GET / HTTP/1.1" 0
"GET / HTTP/1.1
" 400
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

clear && \
echo -e "$WHITE Nginx 컨테이너 생성 중...$NC" && \
make -C nginxContainer > /dev/null 2>&1 && \
echo -e "$WHITE Nginx 컨테이너 생성 완료$NC"

# 응답을 저장할 파일을 생성 (기존 파일이 있으면 내용을 지우고 새로 생성)
echo -n > unexpected_response.txt

# 요청을 반복하면서 테스트
for ((i=0; i<${#requests[@]}; i+=2)); do
    request="${requests[$i]}"
    expected_status_code="${requests[$((i+1))]}"

    # 정수형 변수 예약
    declare -i status_code

    # 요청을 파일로 저장 (-e 옵션으로 이스케이프 문자 처리)
    response=$(echo -e "${request}" | nc -w 1 localhost 8080)
    status_code=$(echo "${response}" | awk 'NR==1 {print $2}')

    # 요청을 보내고 응답 코드 확인
    if [ "$status_code" -eq "$expected_status_code" ]; then
        echo -e "$GREEN 테스트 $((i/2+1)): 성공 (HTTP 상태 코드: $status_code)$NC"
    else
        echo -e "$RED 테스트 $((i/2+1)): 실패 (기대한 HTTP 상태 코드: $expected_status_code, 실제 HTTP 상태 코드: $status_code)$NC"
        echo "--------------------------------------REQUEST--------------------------------------" >> unexpected_response.txt
        echo -e "${request}" >> unexpected_response.txt
        echo "--------------------------------------RESPONSE--------------------------------------" >> unexpected_response.txt
        echo -e "${response}" >> unexpected_response.txt
    fi
    # 나노초 단위로 0.1초 대기
    sleep 0.1
done

echo -e "$WHITE 도커 컨테이너를 삭제 하시겠습니까? (y/n) $NC"
read -t 2 response

if [[ "$response" =~ ^([yY][eE][sS]|[yY])$ ]]; then
    make -C nginxContainer fclean > /dev/null 2>&1
    echo -e "$WHITE 도커 컨테이너 삭제 완료$NC"
fi
