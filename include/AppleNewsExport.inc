<?php

/**
 * @file
 * Class for exporting Drupal content to Apple News.
 */

use ChapterThree\AppleNewsDoc;

/**
 * Base class for exporting Drupal entities.
 */
abstract class AppleNewsExport {

  protected $conf;

  /**
   * Implement __construct().
   *
   * @param array $conf
   *   Configuration @todo.
   */
  public function __construct(array $conf = array()) {
    $this->conf = $conf;
  }

  /**
   * Define exportable entities.
   *
   * @return EntityFieldQuery
   *   Object that will return exportable entities.
   */
  public function query() {
    $query = new EntityFieldQuery();
    $query->addMetaData('account', user_load(1));
    return $query;
  }

  /**
   * Provides a configuration form for this export.
   */
  public function getConfigForm($form, &$form_state) {
    $form = array();

    // @todo submit, etc.
    return $form;
  }

  /**
   * Exports the entity to an Apple Native Document.
   *
   * @param object $entity
   *   An entity to export.
   *
   * @return NULL|\ChapterThree\AppleNewsDoc\Document
   *   A document object, or NULL to skip the export.
   */
  public function export($entity) {
    return NULL;
  }

}
