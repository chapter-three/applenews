<?php

/**
 * @file
 * Destination classes.
 */

use \ChapterThree\AppleNews\Document\Components\Body;
use \ChapterThree\AppleNews\Document\Markdown;

/**
 * Interface for an Apple News source.
 */
interface ApplenewsDestinationInterface {

  /**
   * Administrative label.
   */
  public function label();

  /**
   * Exports to an Apple News Document object.
   *
   * @param mixed $value
   *   Value.
   *
   * @return \ChapterThree\AppleNews\Document\Base
   *   The exported object.
   */
  public function export($value);

}

/**
 * Base class for a configurable Apple News destination.
 */
abstract class ApplenewsDestination implements ApplenewsDestinationInterface {

  /**
   * Settings for this destination.
   *
   * @var array
   *   Associative array keyed on setting key.
   */
  protected $settings;

  /**
   * Weight among a set of destinations.
   *
   * @var int
   */
  public $weight = 0;

  /**
   * ID of parent destination among a set of destinations.
   *
   * @var ApplenewsDestination
   */
  public $pid;

  /**
   * Unique id among a set of destinations.
   *
   * @var int
   */
  public $id;

  /**
   * Implements __sleep().
   */
  public function __sleep() {
    return [
      'settings',
      'weight',
      'id',
      'pid',
    ];
  }

  /**
   * Reference to parent export object.
   *
   * @var AppleNewsExportConfigurable
   */
  public $export;

  /**
   * Depth among a set of destinations.
   *
   * @var int
   */
  public $depth;

  /**
   * Implements __construct().
   */
  public function __construct() {
    $this->settings = [];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    if ($this->id === NULL) {
      return '';
    }
    return '(' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  abstract public function export($value);

  /**
   * Root-only -- can not be a child.
   *
   * @return bool
   *   Is a root.
   */
  public function isRoot() {
    return TRUE;
  }

  /**
   * Leaf-only -- can not have children.
   *
   * @return bool
   *   Is a leaf.
   */
  public function isLeaf() {
    return TRUE;
  }

  /**
   * Provides a configuration form for this destination.
   */
  public function getConfigForm($form, &$form_state) {
    $form['weight'] = [
      '#type' => 'weight',
      '#title' => t('Weight'),
      '#default_value' => $this->weight,
      '#delta' => 50,
    ];

    return $form;
  }

  /**
   * Validation handler for the configuration form for this export.
   */
  public function validateConfigForm($form, &$form_state) {
  }

  /**
   * Submit handler for the configuration form for this export.
   */
  public function submitConfigForm($form, &$form_state) {
    foreach ($this->settings as $key => &$value) {
      $this->settings[$key] = $form_state['values'][$key];
    }
  }

}

/**
 * Body component.
 */
class ApplenewsDestinationBody extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'text' => NULL,
      'format' => 'markdown',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Body') . ' ' . parent::label();
  }

  /**
   * {@inheritdoc}
   */
  public function isRoot() {
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['text'] = [
      '#title' => t('Text'),
      '#type' => 'select',
      '#options' => apple_news_get_source_options($this),
      '#default_value' => $this->settings['text'],
    ];

    $form['format'] = [
      '#title' => t('Format'),
      '#type' => 'select',
      '#default_value' => $this->settings['format'],
      '#options' => [
        '' => t('(None)'),
        'markdown' => t('Markdown'),
      ],
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {
    $identifier = isset($array['identifier']) ? $array['identifier'] : NULL;
    $body = new Body(Markdown::convert($value), $identifier);
    $body->setFormat($this->settings['markdown']);
    return $body;
  }

}
