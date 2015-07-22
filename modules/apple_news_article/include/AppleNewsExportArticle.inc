<?php

/**
 * @file
 * Class for exporting Drupal article content to Apple News.
 */

use ChapterThree\AppleNews\Document;

/**
 * Exports a Drupal article as defined by the default install profile.
 */
class AppleNewsExportArticle extends AppleNewsExport {

  /**
   * {@inheritdoc}
   */
  public function __construct($module, $machine_name, array $arguments, $enabled = FALSE, $highwater = NULL, array $settings = array()) {
    $settings = array_merge([
        'layout' => 'simple',
        'channels' => []
      ],
      $settings
    );
    parent::__construct($module, $machine_name, $arguments, $enabled, $highwater, $settings);
  }

  /**
   * {@inheritdoc}
   */
  public function query() {
    $query = parent::query();

    $query->entityCondition('entity_type', 'node')
      ->entityCondition('bundle', array('article'))
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
      '#type'          => 'select',
      '#title'         => t('Layout'),
      '#default_value' => $this->settings['layout'],
      '#options'       => array(
        'simple'              => t('Simple'),
        'simple_header_image' => t('Simple with Headline above Header Image'),
      ),
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

    // Identifier;
    $id = $wrapper->getIdentifier();

    // Title.
    $title = $wrapper->label();

    // Language code.
    $language = $wrapper->language->value();
    if ($language == LANGUAGE_NONE) {
      $language = language_default();
      $language = $language->language;
    }

    // Body.
    $value = $wrapper->body->raw();
    $value = $value['value'];
    $body = new Document\Components\Body(Document\Markdown::convert($value));
    $body->setFormat('markdown');

    // Image field.
    /** @var EntityDrupalWrapper $file */
    $file = $wrapper->__isset('field_image') ? $wrapper->field_image->value() : FALSE;

    // Byline.
    $byline = t('By !author on !date',
      array(
        '!author' => $wrapper->author->name->value(),
        '!date' => format_date($wrapper->created->value()),
      ),
      array(
        'langcode' => $language,
      ));

    $document = NULL;

    switch ($this->settings['layout']) {

      case 'simple':
        $layout = new Document\Layouts\Layout(7, 1024);
        $document = new Document($id, $title,
          apple_news_language_code($language), $layout);
        // Body.
        $document->addComponent($body);
        // Image.
        if ($file) {
          $image = new Document\Components\Image($file['uri']);
          $document->addComponent($image);
        }
        break;

      case 'header_image':
        $layout = new Document\Layouts\Layout(7, 1024);
        $layout->setMargin(70)
          ->setGutter(40);
        $document = new Document($id, $title,
          apple_news_language_code($language), $layout);
        // Title component.
        $layout = new Document\Layouts\ComponentLayout();
        $layout->setColumnStart(0)->setColumnSpan(7)
          ->setMargin(new Document\Margin(50, 10));
        $document->addComponentLayout('titleLayout', $layout);
        $style = new Document\Styles\ComponentTextStyle();
        $style->setTextAlignment('left')
          ->setFontName('HelveticaNeue-Bold')
          ->setFontSize(4)
          ->setLineHeight(4)
          ->setTextColor('#000');
        $document->addComponentTextStyle('titleStyle', $style);
        $title = new Document\Components\Title($title);
        $title->setLayout('titleLayout', $document)
          ->setTextStyle('titleStyle', $document);
        $document->addComponent($title);
        // Header image.
        if ($file) {
          $layout = new Document\Layouts\ComponentLayout();
          $layout->setColumnStart(0)->setColumnSpan(7)
            ->setIgnoreDocumentMargin()
            ->setMinimumHeight('40vh')
            ->setMargin(new Document\Margin(15, 15));
          $document->addComponentLayout('headerImageLayout', $layout);
          $fill = new Document\Styles\Fills\ImageFill($file['uri']);
          $fill->setFillMode('cover')
            ->setVerticalAlignment('center');
          $style = new Document\Styles\ComponentStyle();
          $style->setFill($fill);
          $header = new Document\Components\Header();
          $header->setLayout('headerImageLayout', $document)
            ->setStyle($style);
          $document->addComponent($header);
        }
        // Byline.
        $layout = new Document\Layouts\ComponentLayout();
        $layout->setColumnStart(0)->setColumnSpan(7)
          ->setMargin(new Document\Margin(15, 15));
        $document->addComponentLayout('authorLayout', $layout);
        $style = new Document\Styles\ComponentTextStyle();
        $style->setTextAlignment('left')
          ->setFontName('HelveticaNeue-Bold')
          ->setFontSize(16)
          ->setTextColor('#000');
        $document->addComponentTextStyle('authorStyle', $style);
        $author = new Document\Components\Author($byline);
        $author->setLayout('authorLayout', $document)
          ->setTextStyle('authorStyle', $document);
        $document->addComponent($author);
        // Body.
        $layout = new Document\Layouts\ComponentLayout();
        $layout->setColumnStart(0)->setColumnSpan(5)
          ->setMargin(new Document\Margin(15, 15));
        $document->addComponentLayout('bodyLayout', $layout);
        $style = new Document\Styles\ComponentTextStyle();
        $style->setTextAlignment('left')
          ->setFontName('Georgia')
          ->setFontSize(18)
          ->setLineHeight(26)
          ->setTextColor('#000');
        $document->addComponentTextStyle('bodyStyle', $style);
        $body->setLayout('bodyLayout', $document)
          ->setTextStyle('bodyStyle', $document);
        $document->addComponent($body);
        break;

    }

    return $document;
  }

}
