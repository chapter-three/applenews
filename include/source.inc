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
   * Implementation-specific types to limit application to.
   *
   * @see ApplenewsDestination::getSourceOptions()
   */
  static public function types();

  /**
   * Defines available attributes for the source.
   *
   * @return array
   *   Associative array of attributes. Values are either:
   *   - String label, if attribute value is provided by this class.
   *   - Array, if attribute value(s) are supplied by another source, with keys:
   *     - label - as above.
   *     - type - a value as returned by types().
   *
   * @see ApplenewsDestination::getSourceOptions()
   */
  static public function attributes();

  /**
   * Dumps data from the source.
   *
   * @return array
   *   Associative array with keys defined in attributes().
   */
  public function export();

}

/**
 * Base class for exporting a Drupal entity.
 */
abstract class ApplenewsSourceEntity implements ApplenewsSourceInterface {

  /**
   * {@inheritdoc}
   */
  static public function label() {
    return t('Entity');
  }

  /**
   * {@inheritdoc}
   */
  static public function attributes() {
    return [
      'identifier' => 'Identifier',
      'label' => 'Label',
      'bundle' => 'Bundle',
      'language' => 'Language',
    ];
  }

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
   *
   * @param object $entity
   *   Entity object.
   */
  public function __construct($entity) {
    $this->wrapper = entity_metadata_wrapper($this->entityType, $entity);
  }

  /**
   * {@inheritdoc}
   */
  public function export() {
    return [
      'identifier' => $this->wrapper->getIdentifier(),
      'label' => $this->wrapper->label(),
      'bundle' => $this->wrapper->getBundle(),
      'language' => $this->wrapper->getPropertyLanguage(),
    ];
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
   * Entity types this class applies to.
   */
  static public function types() {
    return ['node'];
  }

  /**
   * {@inheritdoc}
   */
  static public function attributes() {
    return [
      'identifier' => 'NID',
      'label' => 'Title',
      'bundle' => 'Content Type',
      'author' => [
        'label' => 'Author',
        'type' => 'user',
      ],
    ] + parent::attributes();
  }

  /**
   * {@inheritdoc}
   */
  public function __construct($entity) {
    $this->entityType = 'node';
    parent::__construct($entity);
  }

  /**
   * {@inheritdoc}
   */
  public function export() {
    return [
      'author' => $this->wrapper->author->value(),
    ] + parent::export();
  }

}

/**
 * Exports a Drupal node.
 */
class ApplenewsSourceUser extends ApplenewsSourceEntity {

  /**
   * {@inheritdoc}
   */
  static public function label() {
    return t('User');
  }

  /**
   * Entity types this class applies to.
   */
  static public function types() {
    return array('user');
  }

  /**
   * {@inheritdoc}
   */
  static public function attributes() {
    return [
      'identifier' => 'UID',
      'label' => 'Name',
    ] + parent::attributes();
  }

  /**
   * {@inheritdoc}
   */
  public function __construct($entity) {
    $this->entityType = 'user';
    parent::__construct($entity);
  }

}

/**
 * Exports a Drupal field value.
 *
 * All field source classes need to extend this class.
 */
class ApplenewsSourceField implements ApplenewsSourceInterface {

  /**
   * {@inheritdoc}
   */
  static public function label() {
    return t('Field');
  }

  /**
   * {@inheritdoc}
   */
  static public function attributes() {
    return [
      'value' => 'Value',
    ];
  }

  /**
   * Field types this class applies to.
   */
  static public function types() {
    return [
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
    ];
  }

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
   * Field key.
   *
   * @var string
   */
  public $fieldName;

  /**
   * Implements __construct().
   *
   * @param string $entity_type
   *   Parent entity type.
   * @param string $field_name
   *   Entity key.
   * @param object $entity
   *   Parent entity object.
   */
  public function __construct($entity_type, $field_name, $entity) {
    $this->entityType = $entity_type;
    $this->wrapper = entity_metadata_wrapper($this->entityType, $entity);
    $this->fieldName = $field_name;
  }

  /**
   * {@inheritdoc}
   */
  public function export() {
    try {
      $value = $this->wrapper
        ->{$this->fieldName}->value->value(['decode' => TRUE]);
    }
    catch (EntityMetadataWrapperException $e) {
      $value = '';
    }
    return [
      'value' => $value,
    ];
  }

}
