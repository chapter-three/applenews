<?php

/**
 * @file
 * Class for exporting Drupal article content to Apple News.
 */

use ChapterThree\AppleNews\Document;
use ChapterThree\AppleNews\Document\Layouts\Layout;
use ChapterThree\AppleNews\Document\Components\Body;
use ChapterThree\AppleNews\Document\Components\Image;
use ChapterThree\AppleNews\Document\Markdown;

/**
 * Exports a Drupal article as defined by the default install profile.
 */
class AppleNewsExportArticle extends AppleNewsExport {

  /**
   * {@inheritdoc}
   */
  public function __construct($module, $machine_name, array $arguments, $enabled = FALSE, $highwater = NULL, array $settings = array()) {
    $settings = array(
      'layout' => 'simple',
    ) + $settings;
    parent::__construct($module, $machine_name, $arguments, $enabled,
      $highwater, $settings);
  }

  /**
   * {@inheritdoc}
   */
  public function query() {
    $query = parent::query();

    $query->entityCondition('entity_type', 'node')
      ->propertyCondition('status', NODE_PUBLISHED)
      ->entityOrderBy('entity_id');

    return $query;
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {
    $form = parent::getConfigForm($form, $form_state);

    $form['layout'] = array(
      '#type' => 'select',
      '#title' => t('Layout'),
      '#options' => array(
        'simple' => t('Simple'),
      ),
      '#default_value' => $this->settings['layout'],
    );

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateConfigForm($form, &$form_state) {
    parent::validateConfigForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigForm($form, &$form_state) {
    parent::submitConfigForm($form, $form_state);
    $this->settings['layout'] = $form_state['values']['layout'];
  }

  /**
   * {@inheritdoc}
   */
  public function export($entity) {
    /** @var EntityDrupalWrapper $wrapper */
    $wrapper = entity_metadata_wrapper('node', $entity);

    $lang = $wrapper->language->value();
    if ($lang == LANGUAGE_NONE) {
      $language = language_default();
      $lang = $language->language;
    }

    switch ($this->settings['layout']) {

      case 'simple':
      default:
        $layout = new Layout(7, 1024);
        break;

    }
    $document = new Document($wrapper->getIdentifier(),
      $wrapper->label(), apple_news_language_code($lang), $layout);

    // Body.
    $value = $wrapper->body->raw();
    $value = $value['value'];
    $component = new Body(Markdown::convert($value));
    $component->setFormat('markdown');
    $document->addComponent($component);

    // Image field.
    $value = $wrapper->field_image->value();
    if ($value) {
      $component = new Image($value['uri']);
      $document->addComponent($component);
    }

    return $document;
  }

}
