#!/bin/bash

PORT=8080

# Start a simple HTTP server using netcat
while true; do
    echo "Listening on port $PORT..."
    # Wait for connection, then send HTTP response
    {
        read request
        echo -e "HTTP/1.1 200 OK\r"
        echo -e "Content-Type: text/html\r"
        echo -e "\r"
        echo -e "<!DOCTYPE html>"
        echo -e "<html><head><title>Simple Bash Server</title></head>"
        echo -e "<body><h1>Hello from Bash!</h1><p>This is a simple HTML response.</p></body></html>"
    } | nc -l -p $PORT -q 1
done
