#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os

print("Content-Type: text/html; charset=utf-8\r\n\r\n")

messages_file = "./www/data/messages.txt"
uploads_dir = "./www/uploads"

os.makedirs(os.path.dirname(messages_file), exist_ok=True)
os.makedirs(uploads_dir, exist_ok=True)

# Load messages
messages = []
if os.path.exists(messages_file):
    with open(messages_file, "r", encoding="utf-8") as f:
        for line in f:
            if ":" in line:
                user, msg = line.strip().split(":", 1)
                messages.append((user, msg))

# Load images
images = [
    f for f in os.listdir(uploads_dir)
    if f.lower().endswith(('.png', '.jpg', '.jpeg', '.gif', '.webp'))
]

# Build HTML
print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>🌐 WebServ Feed</title>
    <link rel="stylesheet" href="/css/style.css">
    <style>
        .feed-container {
            display: flex;
            flex-direction: column;
            gap: 20px;
        }
        .feed-item {
            background: rgba(255, 255, 255, 0.1);
            padding: 20px;
            border-radius: 12px;
            box-shadow: 0 2px 6px rgba(0,0,0,0.2);
        }
        .feed-item img {
            max-width: 100%;
            border-radius: 10px;
            margin-top: 10px;
        }
        .feed-item strong {
            color: #c7d2fe;
        }
        h2 {
            margin-bottom: 10px;
        }
        .section-divider {
            border-top: 1px solid rgba(255,255,255,0.2);
            margin: 30px 0;
        }
    </style>
</head>
<body>
    <div class="background">
        <div class="glass-card">
            <header>
                <h1>🌐 Community Feed</h1>
                <a href="/" class="button-link">← Back</a>
            </header>
            <main class="feed-container">
""")

# Show messages first
print("<h2>💬 Messages</h2>")
if messages:
    for user, msg in reversed(messages[-10:]):  # Show last 10
        print(f"""
        <div class="feed-item">
            <p><strong>{user}</strong>: {msg}</p>
        </div>
        """)
else:
    print("<p>No messages yet.</p>")

print('<div class="section-divider"></div>')

# Then show images
print("<h2>🖼 Uploaded Images</h2>")
if images:
    for img in reversed(images[-10:]):  # Show last 10
        print(f"""
        <div class="feed-item">
            <img src="/uploads/{img}" alt="{img}">
            <p>{img}</p>
        </div>
        """)
else:
    print("<p>No images uploaded yet.</p>")

print("""
            </main>
        </div>
    </div>
</body>
</html>
""")
