#!/usr/bin/env python3
import os
import sys

# Use first argument if provided, otherwise default to ./uploads
if len(sys.argv) > 1:
    UPLOAD_FOLDER = sys.argv[1]
else:
    UPLOAD_FOLDER = "./www/uploads"  # default path

# Check if directory exists
if not os.path.isdir(UPLOAD_FOLDER):
    print("Content-Type: text/html\n")
    print(f"<h1>Error: Upload folder '{UPLOAD_FOLDER}' does not exist.</h1>")
    sys.exit(1)

files = [f for f in os.listdir(UPLOAD_FOLDER) if os.path.isfile(os.path.join(UPLOAD_FOLDER, f))]

# Generate HTML
print("Content-Type: text/html\n")
print("<!DOCTYPE html>")
print("<html><head><link rel=\"stylesheet\" href=\"/css/download.css\"></link></head><body><h1>Uploaded Files</h1><ul>")
for file in files:
    # HTML-escape file name for safety
    safe_file = file.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
    print(f'<li><a href="/uploads/{safe_file}" download>{safe_file}</a><button onclick="deleteFile(\'{safe_file}\')">Delete</button></li>')
print('<a href="../index.html">Go back</a>')
print("</ul><script>async function deleteFile(name){const response=await fetch('/uploads/'+encodeURIComponent(name),{method:'DELETE'});alert(response.ok? 'Deleted!' : 'Failed: ' + response.status);if(response.ok){location.reload();}}</script></body></html>")
