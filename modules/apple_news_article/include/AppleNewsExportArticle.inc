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
  public function __construct(array $conf = array()) {
    $this->conf = array(
      'layout' => 'simple',
    ) + $conf;
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
      '#default_value' => $this->conf['layout'],
    );

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export() {
    /** @var EntityDrupalWrapper $wrapper */
    $wrapper = entity_metadata_wrapper('node', $entity);

    $lang = $wrapper->language->value();
    if ($lang == LANGUAGE_NONE) {
      $language = language_default();
      $lang = $language->language;
    }

    switch ($this->conf['layout']) {

      case 'simple':
      default:
        $layout = new AppleNewsDoc\Layouts\Layout(7, 1024);
        break;

    }
    $document = new AppleNewsDoc\Document($wrapper->getIdentifier(),
      $wrapper->label(), apple_news_language_code($lang), $layout);

    // Body.
    $body = $wrapper->body->raw()['value'];
    $component = new AppleNewsDoc\Components\Body(
      AppleNewsDoc\Markdown::convert($body));
    $component->setFormat('markdown');
    $document->addComponent($component);

    return $document;
  }

}
