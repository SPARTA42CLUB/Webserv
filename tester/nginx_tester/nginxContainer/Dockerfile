FROM nginx:1.27.0-alpine

# entrypoint and services
COPY entrypoint.sh .
COPY www/index.py /usr/lib/cgi-bin/index.py
COPY www/forbidden.html /usr/share/nginx/html/forbidden.html
COPY hosts /etc/hosts

RUN apk add --no-cache python3 py3-pip fcgiwrap nodejs npm && \
    mkdir -p /var/log/nginx /usr/share/nginx/html/was && \
    chmod 755 /usr/lib/cgi-bin/index.py && \
    chmod 755 entrypoint.sh && \
    mkdir -p /usr/share/dup

COPY www/app.js /usr/share/nginx/html/was/app.js
COPY www/9090.html /usr/share/nginx/dup/9090.html
COPY www/9091.html /usr/share/nginx/html/9091.html

RUN chown -R nginx:nginx /usr/share/nginx/ && \
    chmod -R 777 /usr/share/nginx/ && \
    chmod 000 /usr/share/nginx/html/forbidden.html

ENTRYPOINT ["sh", "./entrypoint.sh"]
