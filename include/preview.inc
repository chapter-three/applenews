<?php

/**
 * @file
 * Apple News article preview generator class.
 */

namespace ChapterThree\AppleNews\Preview;

/**
 * This class generate downloadable AppleNews formatted documents with
 * it's assets.
 */
class ApplenewsPreview {

  private $destination = '';
  private $realpath = '';
  private $files = [];
  private $article_json = '';
  private $article_id = '';

  /**
   * Implements __construct().
   * Initialize all required variables.
   */
  public function __construct($article_id, array $data = []) {
    $drupal_files_dir = variable_get('file_public_path', conf_path() . '/files');
    $drupal_dir = file_build_uri('apple-news/' . $article_id);
      drupal_mkdir($drupal_dir);
    if (!is_dir($dir)) {
      drupal_mkdir($drupal_dir);
    }
    $this->destination = $drupal_dir;
    $this->realpath = drupal_realpath($drupal_dir);
    $this->files = $data['files'];
    $this->article_json = $data['json'];
    $this->article_id = $article_id;
  }

  /**
   * Save JSON string into article.json file.
   */
  private function saveArticleJSON() {
    file_unmanaged_save_data($this->article_json, $this->destination . '/article.json');
  }

  /**
   * Save article assets into article directory.
   */
  private function saveFiles() {
    foreach ($this->files as $path) {
      $contents = file_get_contents($path);
      $filename = explode("/", $path);
      file_unmanaged_save_data($contents, $this->destination . '/' . $filename[count($filename) - 1]);
    }
  }

  /**
   * Create [article-id].zip file archive.
   */
  private function createArchive() {
    // create object
    $zip = new \ZipArchive();
    // open archive 
    if ($zip->open($this->article_id . '.zip', \ZipArchive::CREATE) !== TRUE) {
      die ("Could not open archive");
    }
    $iterator = new \RecursiveIteratorIterator(new \RecursiveDirectoryIterator($this->realpath));
    foreach ($iterator as $key => $value) {
      $zip->addFile(realpath($key), $key) or die ("ERROR: Could not add file: $key");
    }

    // close and save archive
    $zip->close();
  }

  /**
   * Removes previewable AppleNews article file.
   */
  public function removeArchive($article_id) {
    drupal_rmdir($this->destination);
  }

  /**
   * Archive article assets and generate downloadable zip file.
   */
  public function archive() {
    $this->saveArticleJSON();
    $this->saveFiles();
    $this->createArchive();
  }

}
