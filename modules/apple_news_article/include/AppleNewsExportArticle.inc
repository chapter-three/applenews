<?php

/**
 * @file
 * Class for exporting Drupal article content to Apple News.
 */

use ChapterThree\AppleNewsDoc;

/**
 * Exports a Drupal article as defined by the default install profile.
 */
class AppleNewsExportArticle extends AppleNewsExport {

  /**
   * {@inheritdoc}
   */
  public function export() {
    $document = parent::export();

    $entity = entity_load($this->entityType, array($this->entityId));
    $entity = reset($entity);
    /** @var EntityDrupalWrapper $wrapper */
    $wrapper = entity_metadata_wrapper($this->entityType, $entity);

    // Body.
    $body = $wrapper->body->raw()['value'];
    $component = new AppleNewsDoc\Components\Body(
      AppleNewsDoc\Markdown::convert($body));
    $component->setFormat('markdown');
    $document->addComponent($component);

    return $document;
  }

}
