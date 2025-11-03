<?php
$rootDir = realpath(__DIR__ . '/..');
$uploadDir = $rootDir . '/uploads/';
$postsDir = $rootDir . '/posts/';
$indexFile = $rootDir . '/index.html';

// Make sure required folders exist
if (!is_dir($uploadDir)) mkdir($uploadDir, 0777, true);
if (!is_dir($postsDir)) mkdir($postsDir, 0777, true);

$title = trim($_POST['title'] ?? 'Untitled');
$content = trim($_POST['content'] ?? '');
$imageFile = $_FILES['image'] ?? null;

// Save uploaded image
$uploadedName = '';
if ($imageFile && $imageFile['error'] === UPLOAD_ERR_OK) {
    $tmpName = $imageFile['tmp_name'];
    $fileName = time() . '_' . basename($imageFile['name']);
    $target = $uploadDir . $fileName;
    if (move_uploaded_file($tmpName, $target)) {
        $uploadedName = $fileName;
    }
}

// Generate a unique filename for the new post
$postSlug = preg_replace('/[^a-z0-9]+/i', '-', strtolower($title));
$postFile = $postsDir . $postSlug . '.html';

// Create post HTML
$postHTML = <<<HTML
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <title>{$title} — Runway Bytes</title>
  <link rel="stylesheet" href="../style.css" />
</head>
<body>
  <header>
    <h1><a href="../index.html" style="text-decoration:none;color:#740F2D;">Runway Bytes</a></h1>
    <nav>
      <a href="../index.html">Home</a>
      <a href="../new_post.html">Create Post</a>
    </nav>
  </header>

  <main class="post-content">
    <h2>{$title}</h2>
    <p class="meta">Published on November 3, 2025</p>
HTML;

if ($uploadedName) {
    $postHTML .= '<img src="../uploads/' . htmlspecialchars($uploadedName) . '" alt="' . htmlspecialchars($title) . '" />';
}

$postHTML .= "<p>" . nl2br(htmlspecialchars($content)) . "</p>
    <hr>
    <a class='back-link' href='../index.html'>← Back to Home</a>
  </main>

  <footer>© 2025 Runway Bytes</footer>
</body>
</html>";

file_put_contents($postFile, $postHTML);

// --- Update index.html ---
$index = file_get_contents($indexFile);

// Build the new article card HTML
$imageTag = $uploadedName ? "<img src='uploads/" . htmlspecialchars($uploadedName) . "' alt='" . htmlspecialchars($title) . "' />" : "";
$snippet = htmlspecialchars(substr($content, 0, 120)) . (strlen($content) > 120 ? "..." : "");

$newArticle = <<<HTML
    <article>
      {$imageTag}
      <h2><a href="posts/{$postSlug}.html">{$title}</a></h2>
      <p>{$snippet}</p>
      <form action="/cgi-bin/delete.php" method="POST" style="margin-top:10px;">
        <input type="hidden" name="post" value="{$postSlug}.html">
      <button type="submit"
        onclick="return confirm('Are you sure you want to delete this post?')"
        style="background:#ff6fa5;color:white;border:none;padding:0.5rem 1rem;border-radius:6px;cursor:pointer;">
       Delete
      </button>
     </form>
    </article>

HTML;


// Insert before the closing </main>
$index = preg_replace('/(<\/main>)/', $newArticle . '$1', $index, 1);

file_put_contents($indexFile, $index);

// Confirmation page
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Post Created — Runway Bytes</title>
    <link rel="stylesheet" href="../style.css">
</head>
<body>
<header>
  <h1><a href="../index.html" style="text-decoration:none;color:black;">Runway Bytes</a></h1>
  <nav>
    <a href="../index.html">Home</a>
    <a href="../new_post.html">Create Post</a>
  </nav>
</header>

<main class="post-content">
  <h2>Post Published!</h2>
  <p><strong>Title:</strong> <?= htmlspecialchars($title) ?></p>
  <p>Your post has been added to the homepage 🎉</p>
  <a class="back-link" href="../index.html">← View on Homepage</a>
</main>

<footer>© 2025 Runway Bytes</footer>
</body>
</html>
