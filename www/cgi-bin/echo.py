#!/usr/bin/env python3
import sys, os

# Read POST data from stdin
body = sys.stdin.read(int(os.environ.get("CONTENT_LENGTH", 0) or 0))

print("Content-Type: text/plain\r\n\r\n")
print("Method:", os.environ.get("REQUEST_METHOD"))
print("Body:", body)
