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
    chmod 000 /usr/share/nginx/html/forbidden.html

COPY www/app.js /usr/share/nginx/html/was/app.js

RUN chown -R nginx:nginx /usr/share/nginx/html/was && \
    chmod 777 /usr/share/nginx/html/was/app.js

ENTRYPOINT ["sh", "./entrypoint.sh"]
