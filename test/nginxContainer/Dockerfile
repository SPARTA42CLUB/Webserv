# 기본 nginx 이미지를 사용합니다
FROM nginx:1.27.0-alpine

# 로그 파일을 저장할 디렉토리를 생성합니다
RUN mkdir -p /var/log/nginx

# nginx 실행
CMD ["nginx", "-g", "daemon off;"]
