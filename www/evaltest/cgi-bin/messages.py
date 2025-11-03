#!/usr/bin/env python3
import os, cgi

MSG_FILE = "./www/evaltest/uploads/messages.txt"
form = cgi.FieldStorage()

if "message" in form and "user" in form:
    with open(MSG_FILE, "a") as f:
        f.write(f"{form['user'].value}: {form['message'].value}\n")
    print("Status: 303 See Other")
    print("Location: /cgi-bin/messages.py\r\n")
    print("Content-Type: text/html\r\n\r\n")
    print("<p>✅ Message posted. Redirecting...</p>")
else:
    print("Content-Type: text/html\r\n\r\n")
    print("<html><body><h1>📜 Message Board</h1><pre>")
    if os.path.exists(MSG_FILE):
        with open(MSG_FILE, "r") as f:
            print(f.read())
    else:
        print("(No messages yet)")
    print("</pre><a href='/'>← Back</a></body></html>")
