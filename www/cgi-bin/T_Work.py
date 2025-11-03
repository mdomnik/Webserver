# worker.py
import os
import time

print(f"Worker started with PID {os.getpid()}")
for i in range(5):
    print(f"Worker {os.getpid()} doing task {i + 1}/5...")
    time.sleep(1)
print(f"Worker {os.getpid()} finished.")
