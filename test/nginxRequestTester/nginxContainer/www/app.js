const http = require("http");

const server = http.createServer((req, res) => {
    let body = "";

    // 요청 본문을 받기 위한 이벤트 리스너 설정
    req.on("data", chunk => {
        body += chunk.toString(); // 청크 데이터를 문자열로 변환하여 body에 추가
    });

    // 요청 본문 수신 완료 시 실행되는 이벤트 리스너
    req.on("end", () => {
        // 요청 라인, 헤더, 본문을 포함한 응답 본문 생성
        const responseBody = 
`
Request Line: ${req.method} ${req.url} HTTP/${req.httpVersion}
Headers: ${JSON.stringify(req.headers, null, 2)}
Body: ${body}
`;

        const urlPath = req.url;
        if (urlPath === "/overview") {
            res.end('Welcome to the "overview page" of the nginX project');
        } else if (urlPath === "/api") {
            res.writeHead(200, { "Content-Type": "application/json" });
            res.end(
                JSON.stringify({
                    product_id: "xyz12u3",
                    product_name: "NginX injector",
                })
            );
        } else {
            res.writeHead(200, { "Content-Type": "text/plain" });
            res.end(responseBody);
        }
    });
});

server.listen(3000, "localhost", () => {
    console.log("Listening for request");
});
