from flask import Flask, request, render_template_string
import os

app = Flask(__name__)
UPLOAD_FOLDER = "uploads"
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

HTML_PAGE = """
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>File Upload</title>
</head>
<body>
  <h2>Upload a File</h2>
  <form method="POST" enctype="multipart/form-data">
    <input type="file" name="file" required>
    <button type="submit">Upload</button>
  </form>
  {% if filename %}
  <p>Uploaded: {{ filename }}</p>
  {% endif %}
</body>
</html>
"""

@app.route("/", methods=["GET", "POST"])
def upload_file():
    filename = None
    if request.method == "POST":
        file = request.files.get("file")
        if file:
            filepath = os.path.join(UPLOAD_FOLDER, file.filename)
            file.save(filepath)
            filename = file.filename
    return render_template_string(HTML_PAGE, filename=filename)

if __name__ == "__main__":
    app.run(debug=True)