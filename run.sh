#!/bin/bash

# 스크립트 실행 중 오류 발생 시 중단
set -e

# Docker 이미지 이름
IMAGE_NAME="web_server"

# 컨테이너 이름
CONTAINER_NAME="web_server"

# log 디렉토리를 마운트하기 위해 현재 디렉토리 경로를 가져옵니다
HOST_LOG_DIR=$(pwd)/log

# Docker 이미지 빌드
echo "Building Docker image."
echo "This process takes about 30 seconds..."
docker build -t $IMAGE_NAME . > /dev/null 2>&1 && echo "Docker image built successfully." || echo "Docker image build failed."

# 이전에 실행 중인 동일 이름의 컨테이너가 있으면 중지하고 삭제
if [ $(docker ps -q -f name=$CONTAINER_NAME) ]; then
    echo "Stopping the already running container."
    docker stop $CONTAINER_NAME > /dev/null 2>&1
fi

if [ $(docker ps -aq -f name=$CONTAINER_NAME) ]; then
    docker rm $CONTAINER_NAME > /dev/null 2>&1
fi

# 데몬으로 Docker 컨테이너 실행
echo "Running Docker container..."
docker run -d --name $CONTAINER_NAME -p 8080:8080 -v ${HOST_LOG_DIR}:/usr/src/app/log $IMAGE_NAME > /dev/null 2>&1 && echo "Docker container is up and running." || echo "Failed to run Docker container."
