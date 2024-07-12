#!/bin/bash

# 로그 파일이 저장된 최상위 디렉토리
LOG_DIR="/usr/src/app/log"

# 3일 이상 된 로그 파일 삭제
find "$LOG_DIR" -type f -name "*.log" -mtime +3 -exec rm -f {} \;
