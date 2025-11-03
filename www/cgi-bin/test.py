#!/usr/bin/env python3
import sys
import os

def main():
    # Read environment info (provided by your C++ server)
    method = os.environ.get("REQUEST_METHOD", "")
    content_length = os.environ.get("CONTENT_LENGTH", "0")
    content_type = os.environ.get("CONTENT_TYPE", "")

    try:
        length = int(content_length)
    except ValueError:
        length = 0

    # Read exactly CONTENT_LENGTH bytes from stdin
    body = sys.stdin.read(length) if length > 0 else ""

    # Produce CGI response headers
    print("Content-Type: text/plain\r\n")

    # Body of the response
    print(f"Method: {method}")
    print(f"Content-Type: {content_type}")
    print(f"Content-Length: {length}")
    print(f"Body: {repr(body)}")

if __name__ == "__main__":
    main()
