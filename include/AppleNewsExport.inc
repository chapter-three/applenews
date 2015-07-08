<?php

/**
 * @file
 * Class for exporting Drupal content to Apple News.
 */

use ChapterThree\AppleNewsDoc;

/**
 * Base class for exporting a Drupal entity.
 *
 * @property string $entityType
 * @property int $entityId
 * @property array $config
 */
class AppleNewsExport {

  protected $entityType;
  protected $entityId;

  protected $config;

  /**
   * Implement __construct().
   *
   * @param string $entity_type
   *   Entity Type.
   * @param int $entity_id
   *   Entity ID.
   * @param array $config
   *   Configuration @todo.
   */
  public function __construct($entity_type, $entity_id, array $config = array()) {
    $this->entityType = $entity_type;
    $this->entityId = $entity_id;

    $this->config = array(
      'document_columns' => 7,
      'document_width' => 1024,
    ) + $config;
  }

  /**
   * Provides a configuration form for this export.
   */
  public function getConfigForm($form, &$form_state) {
    return $form;
  }

  /**
   * Exports the entity to an Apple Native Document.
   *
   * @return \ChapterThree\AppleNewsDoc\Document
   *   The document object.
   */
  public function export() {
    $entity = entity_load($this->entityType, array($this->entityId));
    $entity = reset($entity);
    /** @var EntityDrupalWrapper $wrapper */
    $wrapper = entity_metadata_wrapper($this->entityType, $entity);

    $lang = $wrapper->language->value();
    if ($lang == LANGUAGE_NONE) {
      $language = language_default();
      $lang = $language->language;
    }

    $layout = new AppleNewsDoc\Layouts\Layout($this->config['document_columns'],
      $this->config['document_width']);
    $document = new AppleNewsDoc\Document($wrapper->getIdentifier(),
      $wrapper->label(), apple_news_language_code($lang), $layout);

    return $document;
  }

}
