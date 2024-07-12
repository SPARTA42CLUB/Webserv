import http.server
import socketserver

class MyRequestHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        # 요청 메시지 출력
        print("Request Message:")
        print(self.requestline)
        print(self.headers)
        print()

        # 응답 메시지 생성
        self.send_response(200)
        self.send_header("Content-type", "text/plain")
        self.send_header("Content-Length", "13")
        self.end_headers()
        self.wfile.write(b"Hello, World!")

    def finish(self):
        self.request.close()
        super().finish()

PORT = 3000

with socketserver.TCPServer(("", PORT), MyRequestHandler) as httpd:
    print(f"Serving at port {PORT}")
    httpd.serve_forever()
