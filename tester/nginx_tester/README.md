# Nginx
Nginx에 직접 HTTP Request를 보내서 Response를 받아보는 테스트

## Run
```bash
./tester.sh
```

## Installation
```bash
cd nginxContainer && docker-compose up -d
```

## Request Test

### Using curl
```bash
curl -v http://localhost:8080 # Verbose mode
curl -w "%{http_code}\n" -o /dev/null -s http://localhost:8080 # Only status code
curl -v -X GET http://localhost:8080/cgi-bin/
```

### Using telnet
```bash
telnet localhost 8080
GET / HTTP/1.1^M (control v + m + enter)
Host: localhost^M
^M
^M
```

### Using netcat
```bash
echo -e "GET / HTTP/1.1\r\nHost: localhost\r\n" | nc localhost 8080
```

```bash
nc localhost 8080
GET / HTTP/1.1^M (control v + m + enter)
Host: localhost^M
^M
^M
```

## Useful Commands
```bash
docker exec nginx sh # Access Nginx container with shell
nginx -s reload # Reload Nginx
nginx -t # Test Nginx configuration
```

