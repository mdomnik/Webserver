#!/usr/bin/env python3
import sys, os
data = sys.stdin.read()
print("Content-Type: text/plain\r\n")
print(f"Received body:\n{data}")
