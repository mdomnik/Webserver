#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import cgi, os

print("Content-Type: text/html; charset=utf-8\r\n\r\n")

data_file = "./www/data/messages.txt"
form = cgi.FieldStorage()
os.makedirs(os.path.dirname(data_file), exist_ok=True)

username = form.getfirst("username", "").strip()
message = form.getfirst("message", "").strip()

# If message was posted, save and show confirmation
if username and message:
    with open(data_file, "a", encoding="utf-8") as f:
        f.write(f"{username}:{message}\n")
    print(f"""<!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <title>Message Posted</title>
        <link rel="stylesheet" href="/css/style.css">
    </head>
    <body>
        <div class="background">
            <div class="glass-card">
                <h1>✅ Message Sent!</h1>
                <p><strong>{username}</strong>, your message was added successfully.</p>
                <a href="/" class="button-link">← Back to Home</a>
                <a href="/cgi-bin/messages.py" class="button-link">📄 View Message Board</a>
            </div>
        </div>
    </body>
    </html>""")
else:
    # If accessed directly, just show the message list
    print("""<!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <title>💬 Message Board</title>
        <link rel="stylesheet" href="/css/style.css">
    </head>
    <body>
        <div class="background">
            <div class="glass-card">
                <h1>💬 Message Board</h1>
                <a href="/" class="button-link">← Back</a>
                <hr>
    """)

    if os.path.exists(data_file):
        with open(data_file, "r", encoding="utf-8") as f:
            for line in f:
                if ":" in line:
                    user, msg = line.strip().split(":", 1)
                    print(f"<p><strong>{user}</strong>: {msg}</p>")
    else:
        print("<p>No messages yet.</p>")

    print("""
            </div>
        </div>
    </body>
    </html>
    """)
