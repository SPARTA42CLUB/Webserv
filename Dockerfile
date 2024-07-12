# 베이스 이미지로 Ubuntu 사용
FROM ubuntu:latest

#-----------------webserver-----------------#
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

# 빌드된 바이너리를 /usr/local/bin으로 이동
RUN cp webserv /usr/local/bin/webserv && chmod +x /usr/local/bin/webserv

# 포트 설정
EXPOSE 8080
#--------------------end--------------------#

#------------log 파일 cron으로 관리-------------#
# 시간대 설정
ENV TZ=Asia/Seoul
RUN apt-get update && apt-get install -y tzdata
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# cron 및 필요한 패키지 설치
RUN apt-get update && apt-get install -y cron

# 스크립트 복사 및 권한 설정
COPY log/clean_old_logs.sh /usr/local/bin/clean_old_logs.sh
RUN chmod +x /usr/local/bin/clean_old_logs.sh

# crontab 파일 복사
COPY mycron /etc/cron.d/mycron

# crontab 파일 권한 설정
RUN chmod 0644 /etc/cron.d/mycron

# crontab 등록
RUN crontab /etc/cron.d/mycron
#-------------------end------------------#

# cron 및 웹서버 실행
CMD ["sh", "-c", "cron && /usr/local/bin/webserv"]
