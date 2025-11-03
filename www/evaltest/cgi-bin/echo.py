#!/usr/bin/env python3
import cgi
form = cgi.FieldStorage()
body = form.getvalue("name", "Anonymous")
print("Content-Type: text/plain\r\n\r\n")
print("Echo from CGI: " + body)
