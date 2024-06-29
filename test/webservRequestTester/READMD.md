# How to send a requests using netcat
```bash
echo -en "GET / HTTP/1.1\r\nHost: localhost\r\n\n\n" | nc localhost 8080

(echo -en "GET / HTTP/1.1\r\nHost: localhost\r\n\n\n" && sleep 0.1) | nc localhost 8080

nc localhost 8080
GET / HTTP/1.1^M(control v + m + enter)
Host: localhost^M
^M
^M
```
