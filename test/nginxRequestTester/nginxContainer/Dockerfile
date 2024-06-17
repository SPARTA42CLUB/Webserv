FROM nginx:1.27.0-alpine

# entrypoint and services
COPY entrypoint.sh .
COPY www/ /usr/lib/cgi-bin/

RUN apk add --no-cache python3 py3-pip fcgiwrap && \
    mkdir -p /var/log/nginx && \
    chmod 755 /usr/lib/cgi-bin/index.py && \
    chmod 755 entrypoint.sh

ENTRYPOINT ["sh", "./entrypoint.sh"]
