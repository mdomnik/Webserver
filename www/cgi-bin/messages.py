#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import cgi, os

print("Content-Type: text/html; charset=utf-8\r\n\r\n")

data_file = "./www/data/messages.txt"
form = cgi.FieldStorage()

# Ensure folder exists
os.makedirs(os.path.dirname(data_file), exist_ok=True)

# Append new message if form was submitted
if "username" in form and "message" in form:
    username = form["username"].value.strip()
    message = form["message"].value.strip()
    if username and message:
        with open(data_file, "a", encoding="utf-8") as f:
            f.write(f"{username}:{message}\n")

# Build HTML
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
            <header>
                <h1>💬 Message Board</h1>
                <a href="/" class="back-link">← Back</a>
            </header>
            <main>
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
            </main>
        </div>
    </div>
</body>
</html>
""")
