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
    print("Content-Type: text/html")
    print()
    if "file" in params:
        file_path = params["file"][0]
        file_path = pathlib.Path(file_path)
        if file_path.exists():
            try:
                file_path.unlink()
                print("<h1>DELETE request received</h1>")
                print(f"<p>File {file_path} deleted successfully.</p>")
            except Exception as e:
                print("<h1>DELETE request received</h1>")
                print(f"<p>Error deleting file {file_path}: {e}</p>")
        else:
            print("<h1>DELETE request received</h1>")
            print(f"<p>File {file_path} does not exist.</p>")
    else:
        print("<h1>DELETE request received</h1>")
        print("<p>No file specified.</p>")

def main():
    request_method = os.environ.get("REQUEST_METHOD")
    # Print HTTP headers
    print("Content-Type: text/plain")
    if request_method == "HEAD":
        # Content-Length 헤더를 설정하여 본문이 없음을 알림
        print("Content-Length: 0")
        print()  # 헤더와 본문을 구분하는 빈 줄
    else:
        print()
        if request_method in ["GET", "POST"]:
            # Create instance of FieldStorage
            form = cgi.FieldStorage()

            # Get data from fields
            name = form.getvalue("name")
            age = form.getvalue("age")

            # Print the content
            if name and age:
                print(f"Received name: {name}, age: {age}")
            else:
                print("No name or age provided")
        elif request_method == "DELETE":
            handle_delete()

if __name__ == "__main__":
    main()
