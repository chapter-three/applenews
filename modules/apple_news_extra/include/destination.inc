<?php

/**
 * @file
 * Destination classes.
 */

use \ChapterThree\AppleNews\Document\Components;

/**
 * Instagram component.
 */
class ApplenewsExtraDestinationInstagram extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'source' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Instagram') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['source'] = [
      '#title' => t('Link'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['source'],
      '#description' => t('URL format <em>https://instagram.com/p/6Lg1xVkNjJ</em>'),
    ];

    return $form;
  }

  /**
   * Exports to an Apple News Document container object.
   *
   * @param mixed $value
   *   An array of children component objects.
   *
   * @return array
   *   A list of \ChapterThree\AppleNews\Document\Base objects.
   */
  public function export($value) {

    list($key, $values) = $this->getSourceValues($value,
      $this->settings['source']);
    $components = array();
    foreach ($values as $value) {
      $components[] = new Components\Instagram($value[$key]);
    }
    return $components;
  }

}

/**
 * Tweet component.
 */
class ApplenewsExtraDestinationTweet extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'source' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Tweet') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['source'] = [
      '#title' => t('Link'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['source'],
      '#description' => t('URL format <em>https://twitter.com/chapter_three/status/623547179563495424</em>'),
    ];

    return $form;
  }

  /**
   * Exports to an Apple News Document container object.
   *
   * @param mixed $value
   *   An array of children component objects.
   *
   * @return array
   *   A list of \ChapterThree\AppleNews\Document\Base objects.
   */
  public function export($value) {

    list($key, $values) = $this->getSourceValues($value,
      $this->settings['source']);
    $components = array();
    foreach ($values as $value) {
      $components[] = new Components\Tweet($value[$key]);
    }
    return $components;
  }

}
