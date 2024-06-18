FROM nginx:1.27.0-alpine

# entrypoint and services
COPY entrypoint.sh .
COPY www/index.py /usr/lib/cgi-bin/index.py
COPY www/forbidden.html /usr/share/nginx/html/forbidden.html

RUN apk add --no-cache python3 py3-pip fcgiwrap && \
    mkdir -p /var/log/nginx && \
    chmod 755 /usr/lib/cgi-bin/index.py && \
    chmod 755 entrypoint.sh && \
    chmod 000 /usr/share/nginx/html/forbidden.html

ENTRYPOINT ["sh", "./entrypoint.sh"]
