import http.client
import time
import subprocess

UPLOAD_HOST = "localhost"
UPLOAD_PORT = 8081
UPLOAD_PATH = "/upload"
IMAGE_FILE = "testImage.png"
CHUNK_SIZE = 1024

def send_file_in_chunks(file_path, chunk_size, conn):
    with open(file_path, "rb") as f:
        while True:
            chunk = f.read(chunk_size)
            if not chunk:
                break
            actual_chunk_size = len(chunk)
            padded_chunk = chunk + b'\x00' * (chunk_size - actual_chunk_size)
            chunk_size_hex = format(len(padded_chunk), 'x').encode()
            conn.send(b"%s\r\n%s\r\n" % (chunk_size_hex, padded_chunk))
            time.sleep(0.008)

def main():
    conn = http.client.HTTPConnection(UPLOAD_HOST, UPLOAD_PORT)
    
    headers = {
        "Transfer-Encoding": "chunked",
        "Content-Type": "image/png"
    }
    
    conn.request("POST", UPLOAD_PATH, headers=headers)
    # Wait for 1 second before starting to send chunks
    time.sleep(1)
    
    send_file_in_chunks(IMAGE_FILE, CHUNK_SIZE, conn)
    conn.send(b"0\r\n\r\n")
    
    print("File sent successfully.")
    conn.close()
    subprocess.run(["sh", "diff.sh"])

if __name__ == "__main__":
    main()
