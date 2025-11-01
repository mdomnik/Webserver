#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import cgi, os

print("Content-Type: text/html; charset=utf-8\r\n\r\n")

upload_dir = "./www/uploads"
os.makedirs(upload_dir, exist_ok=True)

form = cgi.FieldStorage()

if "file" in form and form["file"].filename:
    filename = os.path.basename(form["file"].filename)
    filepath = os.path.join(upload_dir, filename)
    with open(filepath, "wb") as f:
        f.write(form["file"].file.read())

    print(f"""
    <html><head><meta charset="UTF-8">
    <meta http-equiv="refresh" content="0; URL=/cgi-bin/gallery.py" />
    </head><body>
    <p>✅ Uploaded {filename}. Redirecting...</p>
    </body></html>
    """)
else:
    print("""
    <html><head><meta charset="UTF-8"><title>Upload Failed</title></head>
    <body><p>❌ No file selected.</p><a href="/">Back</a></body></html>
    """)
