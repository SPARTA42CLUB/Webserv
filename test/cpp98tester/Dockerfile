FROM alpine:latest

RUN apk add --no-cache g++ make && \
    mkdir -p /usr/src/app

# 작업 디렉토리 설정
WORKDIR /usr/src/app

# 호스트의 Makefile과 소스 파일을 컨테이너로 복사
COPY testfiles/Makefile .
COPY testfiles/test.* .

# Makefile을 사용하여 a.out 빌드

# 실행 명령어 설정
CMD ["sh", "-c", "make && ./a.out"]
