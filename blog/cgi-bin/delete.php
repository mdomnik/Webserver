<?php
$rootDir   = realpath(__DIR__ . '/..');
$postsDir  = $rootDir . '/posts/';
$indexFile = $rootDir . '/index.html';

$post = basename($_POST['post'] ?? '');
if (!$post) {
    http_response_code(400);
    echo "<h1>Error</h1><p>Missing post filename.</p>";
    exit;
}

// Delete the post file
$postPath = $postsDir . $post;
if (file_exists($postPath)) unlink($postPath);

// ---- Safely remove its card from index.html ----
$indexHtml = file_get_contents($indexFile);

// Load the DOM parser (no regex!)
$dom = new DOMDocument();
libxml_use_internal_errors(true); // suppress HTML5 warnings
$dom->loadHTML($indexHtml);
libxml_clear_errors();

// Find all <article> elements
$articles = $dom->getElementsByTagName('article');
$toRemove = null;

// Loop to find the one that links to the deleted post
foreach ($articles as $article) {
    $links = $article->getElementsByTagName('a');
    foreach ($links as $a) {
        if (strpos($a->getAttribute('href'), "posts/$post") !== false) {
            $toRemove = $article;
            break 2;
        }
    }
}

// Remove that one article
if ($toRemove && $toRemove->parentNode) {
    $toRemove->parentNode->removeChild($toRemove);
}

// Save a backup just in case
copy($indexFile, $indexFile . '.bak');

// Save the cleaned HTML back
file_put_contents($indexFile, $dom->saveHTML());
?>
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Post Deleted — Runway Bytes</title>
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
    <h2>Post Deleted</h2>
    <p>The post <strong><?= htmlspecialchars($post) ?></strong> was removed successfully.</p>
    <a class="back-link" href="../index.html">← Back to Home</a>
  </main>

  <footer>© 2025 Runway Bytes</footer>
</body>
</html>
