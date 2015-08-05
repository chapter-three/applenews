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
  private $files_names = [];
  private $article_json = '';
  private $archive_path = '';

  /**
   * Implements __construct().
   * Initialize all required variables.
   */
  public function __construct($entity_id, array $data = []) {

    $drupal_files_dir = variable_get('file_public_path', conf_path() . '/files');
    $drupal_dir = file_build_uri('apple-news/' . $entity_id);

    $this->destination = $drupal_dir;
    $this->realpath = drupal_realpath($drupal_dir);
    $this->files = $data['files'];
    foreach ($data['files'] as $file) {
      $this->files_names[] = basename($file);
    }
    $this->article_json = $data['json'];
    $this->archive_path = drupal_realpath(file_build_uri('apple-news/' . $entity_id . '.zip'));

    $this->removeArchive();
    file_prepare_directory($drupal_dir, FILE_CREATE_DIRECTORY);

  }

  /**
   * Save JSON string into article.json file.
   */
  private function saveArticleJSON() {
    file_unmanaged_save_data($this->article_json, $this->destination . '/article.json');
    $this->files_names[] = 'article.json';
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
    if ($zip->open($this->archive_path, \ZipArchive::CREATE) !== TRUE) {
      die ("Could not open archive");
    }

    $files = array_values(array_filter(scandir($this->realpath), function($file) {
      return !is_dir($file);
    }));

    foreach ($files as $file) {
      if (in_array($file, $this->files_names)) {
        $zip->addFile($this->realpath . '/' . $file, $file) or die ("ERROR: Could not add file: $file");
      }
    }
    // close and save archive
    $zip->close();
  }

  /**
   * Removes previewable AppleNews article file.
   */
  public function removeArchive() {
  
    if (is_dir($this->realpath)) {

      $files = array_values(array_filter(scandir($this->realpath), function($file) {
        return !is_dir($file);
      }));

      foreach ($files as $file) {
        file_unmanaged_delete($this->destination . '/' . $file);
      }
      file_unmanaged_delete_recursive($this->destination);
    }
  }

  /**
   * Archive article assets and generate downloadable zip file.
   */
  public function archive() {
    $this->saveArticleJSON();
    $this->saveFiles();
    $this->createArchive();

    // Remove directory but the archived file.
    $this->removeArchive();
  }

}
