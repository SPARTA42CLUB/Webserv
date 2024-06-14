# Nginx
Nginx에 직접 HTTP Request를 보내서 Response를 받아보는 테스트

## Installation
```bash
make
```

## Request Test

### Using curl
```bash
curl http://localhost:8080
```

### Using telnet
```bash
telnet localhost 8080
GET / HTTP/1.1
Host: localhost

# Press Enter twice
```

### Using netcat
```bash
echo -e "GET / HTTP/1.1\nHost: localhost\n" | nc localhost 8080
```


## Useful Commands
```bash
docker exec nginx sh # Access Nginx container with shell
nginx -s reload # Reload Nginx
nginx -t # Test Nginx configuration
```
