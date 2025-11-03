#!/bin/bash
# ==========================================================
# 🌐 Webserv Evaluation (curl-only) Test Suite
# Tests your server with www/evaltest + ConfigFiles/evaltest.conf
# ==========================================================

SERVER=./webserv
CONF=ConfigFiles/evaltest.conf
PORT=8080
HOST=http://127.0.0.1:$PORT

echo "🧩 Webserv Curl-only Evaluation Tests"
echo "==========================================="

# --- Helper functions ---
start_server() {
    echo "🟢 Starting server..."
    pkill -9 webserv 2>/dev/null
    $SERVER $CONF > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
}

stop_server() {
    echo "🔴 Stopping server..."
    kill $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
    sleep 1
}

check() {
    URL="$1"
    EXPECT="$2"
    CODE=$(curl -s -o /dev/null -w "%{http_code}" "$HOST$URL")
    if [ "$CODE" = "$EXPECT" ]; then
        echo "✅ [$EXPECT] $URL"
    else
        echo "❌ $URL → expected $EXPECT, got $CODE"
    fi
}

echo "🚀 Launching tests..."
start_server

# ==========================================================
# 1️⃣ Basic GET tests
# ==========================================================
echo ""
echo "1️⃣ Basic GET tests"
check "/" 200
check "/index.html" 200
check "/notfound.html" 404

# ==========================================================
# 2️⃣ Error pages
# ==========================================================
echo ""
echo "2️⃣ Error page rendering"
check "/errors/403.html" 200
check "/errors/404.html" 200
check "/errors/405.html" 200
check "/errors/500.html" 200

# ==========================================================
# 3️⃣ Method restrictions
# ==========================================================
echo ""
echo "3️⃣ Method restriction tests"
check "/" 200
curl -s -o /dev/null -w "HTTP %{http_code}\n" -X POST "$HOST/" | grep 405 && echo "✅ 405 handled OK" || echo "❌ 405 failed"
curl -s -o /dev/null -w "HTTP %{http_code}\n" -X PUT "$HOST/" | grep 405 && echo "✅ PUT denied" || echo "❌ PUT failed"

# ==========================================================
# 4️⃣ File upload tests
# ==========================================================
echo ""
echo "4️⃣ Uploading files to /uploads/"
TEST_FILE="/tmp/upload_test.txt"
echo "Test upload from curl at $(date)" > $TEST_FILE
RESP=$(curl -s -o /dev/null -w "%{http_code}" -F "file=@$TEST_FILE" "$HOST/uploads/upload.txt")
if [ "$RESP" = "201" ] || [ "$RESP" = "200" ]; then
    echo "✅ File uploaded successfully"
else
    echo "❌ Upload failed ($RESP)"
fi

echo "📂 Checking that file exists..."
curl -s -o /dev/null -w "%{http_code}" "$HOST/uploads/upload.txt" | grep -q 200 && echo "✅ File accessible" || echo "❌ File missing"

# ==========================================================
# 5️⃣ DELETE tests
# ==========================================================
echo ""
echo "5️⃣ DELETE request"
curl -s -X DELETE "$HOST/uploads/upload.txt" -o /dev/null -w "HTTP %{http_code}\n" | grep 200 && echo "✅ DELETE OK" || echo "❌ DELETE failed"

# ==========================================================
# 6️⃣ CGI execution tests
# ==========================================================
echo ""
echo "6️⃣ CGI execution"
echo "- GET hello.py"
curl -s -o /tmp/cgi_hello.html "$HOST/cgi-bin/hello.py"
grep -q "Hello" /tmp/cgi_hello.html && echo "✅ hello.py works" || echo "❌ CGI hello.py failed"

echo "- POST echo.py"
curl -s -X POST -d "name=maciej" "$HOST/cgi-bin/echo.py" | grep -q "maciej" && echo "✅ echo.py works" || echo "❌ CGI echo.py failed"

echo "- Message board write/read"
curl -s -X POST -d "user=dominik&message=HelloServer" "$HOST/cgi-bin/messages.py" > /dev/null
curl -s "$HOST/cgi-bin/messages.py" | grep -q "HelloServer" && echo "✅ Message posted OK" || echo "❌ Message missing"

# ==========================================================
# 7️⃣ Error responses
# ==========================================================
echo ""
echo "7️⃣ Error responses"
check "/doesnotexist.html" 404
curl -s -X PATCH "$HOST/" -o /dev/null -w "%{http_code}\n" | grep 405 && echo "✅ PATCH correctly denied" || echo "❌ PATCH failed"

# ==========================================================
# 8️⃣ Autoindex / directory browsing
# ==========================================================
echo ""
echo "8️⃣ Directory listing"
curl -s "$HOST/uploads/" | grep -q "<html" && echo "✅ Autoindex enabled" || echo "❌ Autoindex missing"

# ==========================================================
# 9️⃣ Chunked Transfer Encoding simulation
# ==========================================================
echo ""
echo "9️⃣ Transfer-Encoding: chunked"
(
  echo -en "POST /cgi-bin/echo.py HTTP/1.1\r\n"
  echo -en "Host: 127.0.0.1\r\n"
  echo -en "Transfer-Encoding: chunked\r\n\r\n"
  echo -en "5\r\nHello\r\n6\r\n World\r\n0\r\n\r\n"
) | nc 127.0.0.1 $PORT | grep -q "Hello World" && echo "✅ Chunked handled" || echo "❌ Chunked failed"

# ==========================================================
# 🔟 Cleanup
# ==========================================================
stop_server
echo ""
echo "✅ All curl-based evaluation tests complete!"
