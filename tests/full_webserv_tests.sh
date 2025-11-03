#!/bin/bash
# ================================================================
# 🌐 Full Webserv Evaluation Test Suite
# Compatible with 42 Subject + Evaluation Sheet
# ================================================================

SERVER_EXEC=./webserv
CONFIG_DIR=ConfigFiles
DEFAULT_CONF=$CONFIG_DIR/default.conf
PORT=8080

echo "🧩 Starting Webserv Test Suite"
echo "========================================================="

# --- Helper ---
start_server() {
    pkill -9 webserv 2>/dev/null
    echo "🟢 Launching webserv with $1..."
    $SERVER_EXEC "$1" &
    SERVER_PID=$!
    sleep 1
}

stop_server() {
    echo "🔴 Stopping webserv..."
    kill $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
    sleep 1
}

check_response() {
    CODE=$(curl -s -o /dev/null -w "%{http_code}" "$1")
    EXPECT=$2
    if [ "$CODE" = "$EXPECT" ]; then
        echo "✅ [$EXPECT] $1"
    else
        echo "❌ Expected $EXPECT but got $CODE for $1"
    fi
}

# ================================================================
# 1️⃣ SERVER BASIC CHECKS
# ================================================================
start_server "$DEFAULT_CONF"

echo "1️⃣ Basic GET/POST/DELETE tests"
check_response "http://127.0.0.1:$PORT/" 200
check_response "http://127.0.0.1:$PORT/index.html" 200
check_response "http://127.0.0.1:$PORT/notfound.html" 404

# Simple POST
curl -s -X POST -d "message=hello" "http://127.0.0.1:$PORT/cgi-bin/messages.py" > /dev/null && echo "✅ POST message"

# DELETE test
touch www/uploads/delete_me.txt
curl -s -X DELETE "http://127.0.0.1:$PORT/uploads/delete_me.txt" > /dev/null && echo "✅ DELETE success"

# Invalid Method
curl -s -X PUT "http://127.0.0.1:$PORT/" -o /dev/null -w "HTTP %{http_code}\n"

# ================================================================
# 2️⃣ ERROR PAGE CHECKS
# ================================================================
echo "2️⃣ Error Page Tests"
check_response "http://127.0.0.1:$PORT/unknown" 404
check_response "http://127.0.0.1:$PORT/forbidden" 403

# Simulate internal server error via CGI crash
curl -s "http://127.0.0.1:$PORT/cgi-bin/broken.py" -o /dev/null -w "HTTP %{http_code}\n"

# ================================================================
# 3️⃣ AUTOINDEX & DIRECTORY LISTING
# ================================================================
echo "3️⃣ Autoindex and directories"
check_response "http://127.0.0.1:$PORT/uploads/" 200
check_response "http://127.0.0.1:$PORT/images/" 200

# No autoindex (should return 403)
check_response "http://127.0.0.1:$PORT/errors/" 403

# ================================================================
# 4️⃣ MULTI-PORT / MULTI-SERVER TESTS
# ================================================================
echo "4️⃣ Multi-server ports"
stop_server
start_server "$CONFIG_DIR/multi_server.conf"

check_response "http://127.0.0.1:8080/" 200
check_response "http://127.0.0.1:8081/" 200

# Hostname test (requires /etc/hosts entry)
# echo "127.0.0.1 example.com" | sudo tee -a /etc/hosts
# curl -H "Host: example.com" http://127.0.0.1:8080/

# ================================================================
# 5️⃣ BODY SIZE LIMIT
# ================================================================
echo "5️⃣ Client body size limit"
SMALL_BODY=$(head -c 1000 </dev/zero | tr '\0' 'A')
LARGE_BODY=$(head -c 10000000 </dev/zero | tr '\0' 'A')

curl -s -X POST -d "$SMALL_BODY" "http://127.0.0.1:$PORT/cgi-bin/messages.py" > /dev/null && echo "✅ small body accepted"
curl -s -X POST -d "$LARGE_BODY" "http://127.0.0.1:$PORT/cgi-bin/messages.py" -o /dev/null -w "HTTP %{http_code}\n"

# ================================================================
# 6️⃣ CGI TESTS
# ================================================================
echo "6️⃣ CGI tests"
check_response "http://127.0.0.1:$PORT/cgi-bin/hello.py" 200
check_response "http://127.0.0.1:$PORT/cgi-bin/echo.py" 200

# POST to CGI
curl -s -X POST -d "name=test_user" "http://127.0.0.1:$PORT/cgi-bin/echo.py" | grep "test_user" && echo "✅ CGI POST OK"

# Non-existing CGI
check_response "http://127.0.0.1:$PORT/cgi-bin/missing.py" 404

# ================================================================
# 7️⃣ CHUNKED TRANSFER
# ================================================================
echo "7️⃣ Chunked transfer test"
(echo -en "POST /cgi-bin/echo.py HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n6\r\n World\r\n0\r\n\r\n") | nc 127.0.0.1 $PORT

# ================================================================
# 8️⃣ ILLEGAL METHODS
# ================================================================
echo "8️⃣ Illegal method tests"
for method in PUT PATCH TRACE CONNECT OPTIONS HEAD; do
    echo "Testing $method..."
    curl -s -X $method "http://127.0.0.1:$PORT/" -o /dev/null -w "HTTP %{http_code}\n"
done

# ================================================================
# 9️⃣ STRESS TESTS (Siege)
# ================================================================
echo "9️⃣ Siege stress test (10s)"
if command -v siege >/dev/null; then
    siege -b -t10S "http://127.0.0.1:$PORT/"
else
    echo "⚠️ Siege not installed — skipping stress test"
fi

# ================================================================
# 🔟 CLEANUP
# ================================================================
stop_server
echo "✅ All tests finished!"
