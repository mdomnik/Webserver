#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os

print("Content-Type: text/html; charset=utf-8\r\n\r\n")

uploads_dir = "./www/uploads"
os.makedirs(uploads_dir, exist_ok=True)

print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>🖼 Uploaded Images</title>
    <link rel="stylesheet" href="/css/style.css">
    <style>
        .gallery-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
        }
        .gallery-item {
            text-align: center;
            background: rgba(255,255,255,0.1);
            padding: 10px;
            border-radius: 10px;
        }
        .gallery-item img {
            max-width: 100%;
            border-radius: 10px;
            box-shadow: 0 2px 6px rgba(0,0,0,0.2);
        }
    </style>
</head>
<body>
    <div class="background">
        <div class="glass-card">
            <header>
                <h1>🖼 Uploaded Images</h1>
                <a href="/" class="back-link">← Back</a>
            </header>
            <main class="gallery-grid">
""")

for filename in os.listdir(uploads_dir):
    if filename.lower().endswith(('.png', '.jpg', '.jpeg', '.gif', '.webp')):
        print(f"""
        <div class="gallery-item">
            <img src="/uploads/{filename}" alt="{filename}">
            <p>{filename}</p>
        </div>
        """)

print("""
            </main>
        </div>
    </div>
</body>
</html>
""")
