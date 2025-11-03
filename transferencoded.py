import socket

HOST = "localhost"
PORT = 8079

# Create a raw TCP socket
with socket.create_connection((HOST, PORT)) as s:
    # Send the HTTP headers first
    headers = (
        "POST / HTTP/1.1\r\n"
        f"Host: {HOST}\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
    )
    s.sendall(headers.encode())

    # Send chunk 1 (length 5)
    s.sendall(b"5\r\nHello\r\n")

    # Send chunk 2 (length 6)
    s.sendall(b"6\r\n World\r\n")

    # Send final zero-length chunk to end the body
    s.sendall(b"0\r\n\r\n")

    # Read and print the response
    response = s.recv(4096)
    print(response.decode(errors="replace"))
