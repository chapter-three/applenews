<?php

/**
 * @file
 * Source classes.
 */

/**
 * Exports a Drupal link_field.
 */
class ApplenewsExtraSourceFieldLink extends ApplenewsSourceField {

  /**
   * {@inheritdoc}
   */
  static public function label() {
    return t('Link');
  }

  /**
   * {@inheritdoc}
   */
  static public function attributes() {
    return [
      'url' => 'URL',
    ];
  }

  /**
   * Field types this class applies to.
   */
  static public function types() {
    return [
      'link_field',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function export($value = NULL) {
    $return = [
      'url' => '',
    ];
    if (!isset($value)) {
      try {
        $value = $this->wrapper->{$this->fieldName};
      }
      catch (EntityMetadataWrapperException $e) {
        return $return;
      }
    }
    $v = $value->value();
    if (!empty($v['url'])) {
      $return['url'] = $v['url'];
    }
    return $return;
  }

}
