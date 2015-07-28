<?php

/**
 * @file
 * Source classes.
 */

/**
 * Interface for an Apple News source.
 */
interface ApplenewsSourceInterface {

  /**
   * Administrative label.
   */
  static public function label();

  /**
   * List of field types handled by this handler.
   *
   * @var array|NULL
   *   NULL if this class does not apply to fields, otherwise a list of string
   *   field types as defined in hook_field_info() implementations.
   */
  static public function fieldTypes();

  /**
   * Defines available values.
   *
   * @return array
   *   Associative array of field names.
   */
  static public function values();

  /**
   * Dumps data from the source.
   *
   * @param mixed $value
   *   A source value.
   *
   * @return array
   *   Associative array keyed on same keys as values().
   */
  public function export($value);

}

/**
 * Exports a Drupal entity.
 */
class ApplenewsSourceEntity implements ApplenewsSourceInterface {

  /**
   * Entity type.
   *
   * @var string
   */
  public $entityType;

  /**
   * Metadata wrapper for the entity.
   *
   * @var EntityDrupalWrapper
   */
  public $wrapper;

  /**
   * Implements __construct().
   */
  public function __construct($entity_type) {
    $this->entityType = $entity_type;
  }

  /**
   * Implements __sleep().
   *
   * Exclude $this->wrapper.
   */
  public function __sleep() {
    return array(
      'entityType',
    );
  }

  /**
   * {@inheritdoc}
   */
  static public function label() {
    return t('Entity');
  }

  /**
   * {@inheritdoc}
   */
  static public function fieldTypes() {
    return NULL;
  }

  /**
   * {@inheritdoc}
   */
  static public function values() {
    return array(
      'identifier' => 'Identifier',
      'label' => 'Label',
      'bundle' => 'Bundle',
      'language' => 'Language',
    );
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {
    $this->wrapper = entity_metadata_wrapper($this->entityType, $value);
    return array(
      'identifier' => $this->wrapper->getIdentifier(),
      'label' => $this->wrapper->label(),
      'bundle' => $this->wrapper->getBundle(),
      'language' => $this->wrapper->getPropertyLanguage(),
    );
  }

}

/**
 * Exports a Drupal node.
 */
class ApplenewsSourceNode extends ApplenewsSourceEntity {

  /**
   * {@inheritdoc}
   */
  static public function label() {
    return t('Node');
  }

  /**
   * {@inheritdoc}
   */
  static public function values() {
    return array(
      'identifier' => 'NID',
      'label' => 'Title',
      'bundle' => 'Content Type',
    ) + parent::values();
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {
    $export = parent::export($value);
    return $export;
  }

}

/**
 * Exports a Drupal field.
 */
class ApplenewsSourceField implements ApplenewsSourceInterface {

  /**
   * Parent entity instantiating this field.
   *
   * @var ApplenewsSourceEntity
   */
  public $parentEntity;

  /**
   * Field key.
   *
   * @var string
   */
  public $fieldName;

  /**
   * {@inheritdoc}
   */
  static public function label() {
    return t('Field');
  }

  /**
   * {@inheritdoc}
   */
  static public function values() {
    return array(
      'value' => 'Value',
    );
  }

  /**
   * Implements __construct().
   */
  public function __construct(ApplenewsSourceEntity $parent_entity, $field_name) {
    $this->parentEntity = $parent_entity;
    $this->fieldName = $field_name;
  }

  /**
   * {@inheritdoc}
   */
  static public function fieldTypes() {
    return array(
      'text',
      'text_long',
      'text_with_summary',
      'value',
      'list',
      'list_boolean',
      'list_integer',
      'list_float',
      'list_text',
      'number_integer',
      'number_decimal',
      'number_float',
    );
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {
    /** @var EntityMetadataWrapper $field */
    $field = $this->parentEntity->{$this->fieldName};
    return array(
      'value' => $field->value()->value(array('decode' => TRUE)),
    );
  }

}
