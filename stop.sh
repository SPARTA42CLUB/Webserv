#!/bin/bash

# 컨테이너 이름
CONTAINER_NAME="web_server"

# 컨테이너가 실행 중인지 확인
if [ $(docker ps -q -f name=$CONTAINER_NAME) ]; then
    echo "Stopping the running container."
    echo "This process takes a few seconds..."
    docker stop $CONTAINER_NAME > /dev/null 2>&1
    echo "Container stopped."
else
    echo "No running container found."
	exit 0
fi

# 컨테이너가 존재하는지 확인하고 삭제
if [ $(docker ps -aq -f name=$CONTAINER_NAME) ]; then
    echo "Removing the container..."
    docker rm $CONTAINER_NAME > /dev/null 2>&1
    echo "Container removed."
fi
