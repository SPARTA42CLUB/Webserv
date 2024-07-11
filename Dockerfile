# 베이스 이미지로 Ubuntu 사용
FROM ubuntu:latest

# 필수 패키지 설치
RUN apt-get update && \
    apt-get install -y build-essential && \
    apt-get install -y build-essential libc6-dev

# 작업 디렉토리 설정
WORKDIR /usr/src/app

# 소스 코드 복사
COPY . .

# 웹서버 빌드
RUN make

# 포트 설정
EXPOSE 8080

# 웹서버 실행
CMD ["./webserv"]
