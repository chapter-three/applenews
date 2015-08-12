<?php

/**
 * @file
 * Destination classes.
 */

use \ChapterThree\AppleNews\Document\Components;
use \ChapterThree\AppleNews\Document\Styles;

/**
 * Image header component.
 */
class ApplenewscomponentsDestinationHeaderImage extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'file' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Header Image') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['file'] = [
      '#title' => t('File'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['file'],
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {
    $source = $this->getSource($value, $this->settings['file']);
    $key = $this->getSourceKey($this->settings['file']);
    $file = $source->export()[$key];
    $fill = new Styles\Fills\ImageFill($this->export->addAsset($file->uri));
    $fill->setFillMode('cover')->setVerticalAlignment('bottom');
    $style = new Styles\ComponentStyle();
    $style->setFill($fill);
    $header = new Components\Header();
    $header->setStyle($style);
    return [$header];
  }

}
