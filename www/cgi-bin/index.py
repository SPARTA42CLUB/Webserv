#!/usr/bin/env python3
import os
import cgi
import cgitb
import pathlib
import warnings

warnings.filterwarnings("ignore", category=DeprecationWarning)

# Enable CGI error reporting
cgitb.enable()


def handle_delete():
    query_string = os.environ.get("QUERY_STRING", "")
    params = cgi.parse_qs(query_string)
    status_line = "HTTP/1.1 200 OK\r\n"
    headers = "Content-Type: text/html\r\n"
    body = ""
    if "file" in params:
        file_path = params["file"][0]
        file_path = pathlib.Path(file_path)
        if file_path.exists():
            try:
                file_path.unlink()
                body = "<h1>DELETE request received</h1><p>File {file_path} deleted successfully.</p>"
            except Exception as e:
                body = "<h1>DELETE request received</h1><p>Error deleting file {file_path}: {e}</p>"
        else:
            body = "<h1>DELETE request received</h1><p>File {file_path} does not exist.</p>"
    else:
        body = "<h1>DELETE request received</h1><p>No file specified.</p>"
    return status_line, headers, body


def main():
    request_method = os.environ.get("REQUEST_METHOD")
    if request_method == "HEAD":
        status_line = "HTTP/1.1 200 OK\r\n"
        headers = "Content-Type: text/plain\r\n"
        headers += "Content-Length: 0\r\n"
        body = ""
    else:
        if request_method in ["GET", "POST"]:
            form = cgi.FieldStorage()
            name = form.getvalue("name")
            age = form.getvalue("age")
            body = "<html><body>"
            if name and age:
                body += f"<h1>Received name: {name}, age: {age}</h1>"
            else:
                body += "<h1>GET/POST request received</h1>"
            body += "</body></html>"
            status_line = "HTTP/1.1 200 OK\r\n"
            headers = "Content-Type: text/html\r\n"
            headers += f"Content-Length: {len(body)}\r\n"
        elif request_method == "DELETE":
            status_line, headers, body = handle_delete()
        else:
            status_line = "HTTP/1.1 404 Not Found\r\n"
            headers = "Content-Type: text/html\r\n"
            body = "<html><body><h1>404 Not Found</h1></body></html>"

    print(status_line + headers + "\r\n" + body)

if __name__ == "__main__":
    main()
