<?php

/**
 * @file
 * ApplenewsExportArticle class.
 */

use ChapterThree\AppleNews\Document;

/**
 * Exports a Drupal article as defined by the default install profile.
 */
class ApplenewsExportArticle extends ApplenewsExport {

  /**
   * {@inheritdoc}
   */
  public function __construct($module, $machine_name, array $arguments) {
    parent::__construct($module, $machine_name, $arguments);
    $this->settings += [
      'layout' => 'simple',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function query() {
    return parent::query()
      ->entityCondition('entity_type', 'node')
      ->entityCondition('bundle', ['article'])
      ->propertyCondition('status', NODE_PUBLISHED)
      ->entityOrderBy('entity_id');
  }

  /**
   * {@inheritdoc}
   */
  public function entityTypes() {
    return ['node'];
  }

  /**
   * {@inheritdoc}
   */
  public function bundles() {
    return ['article'];
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {
    $form = parent::getConfigForm($form, $form_state);

    $form['layout'] = [
      '#type'          => 'select',
      '#title'         => t('Layout'),
      '#default_value' => $this->settings['layout'],
      '#options'       => [
        'simple'              => t('Simple'),
        'simple_header_image' => t('Simple with Headline above Header Image'),
      ],
    ];

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
      [
        '!author' => $wrapper->author->name->value(),
        '!date' => format_date($wrapper->created->value()),
      ],
      [
        'langcode' => $language,
      ]
    );

    switch ($this->settings['layout']) {

      case 'simple':
        $layout = new Document\Layouts\Layout(7, 1024);
        $this->document = new Document($id, $title,
          apple_news_language_code($language), $layout);
        // Body.
        $this->document->addComponent($body);
        // Image.
        if ($file) {
          $image = new Document\Components\Image($file['uri']);
          $this->document->addComponent($image);
        }
        break;

      case 'header_image':
        $layout = new Document\Layouts\Layout(7, 1024);
        $layout->setMargin(70)
          ->setGutter(40);
        $this->document = new Document($id, $title,
          apple_news_language_code($language), $layout);
        // Title component.
        $layout = new Document\Layouts\ComponentLayout();
        $layout->setColumnStart(0)->setColumnSpan(7)
          ->setMargin(new Document\Margin(50, 10));
        $this->document->addComponentLayout('titleLayout', $layout);
        $style = new Document\Styles\ComponentTextStyle();
        $style->setTextAlignment('left')
          ->setFontName('HelveticaNeue-Bold')
          ->setFontSize(4)
          ->setLineHeight(4)
          ->setTextColor('#000');
        $this->document->addComponentTextStyle('titleStyle', $style);
        $title = new Document\Components\Title($title);
        $title->setLayout('titleLayout', $this->document)
          ->setTextStyle('titleStyle', $this->document);
        $this->document->addComponent($title);
        // Header image.
        if ($file) {
          $layout = new Document\Layouts\ComponentLayout();
          $layout->setColumnStart(0)->setColumnSpan(7)
            ->setIgnoreDocumentMargin()
            ->setMinimumHeight('40vh')
            ->setMargin(new Document\Margin(15, 15));
          $this->document->addComponentLayout('headerImageLayout', $layout);
          $fill = new Document\Styles\Fills\ImageFill($file['uri']);
          $fill->setFillMode('cover')
            ->setVerticalAlignment('center');
          $style = new Document\Styles\ComponentStyle();
          $style->setFill($fill);
          $header = new Document\Components\Header();
          $header->setLayout('headerImageLayout', $this->document)
            ->setStyle($style);
          $this->document->addComponent($header);
        }
        // Byline.
        $layout = new Document\Layouts\ComponentLayout();
        $layout->setColumnStart(0)->setColumnSpan(7)
          ->setMargin(new Document\Margin(15, 15));
        $this->document->addComponentLayout('authorLayout', $layout);
        $style = new Document\Styles\ComponentTextStyle();
        $style->setTextAlignment('left')
          ->setFontName('HelveticaNeue-Bold')
          ->setFontSize(16)
          ->setTextColor('#000');
        $this->document->addComponentTextStyle('authorStyle', $style);
        $author = new Document\Components\Author($byline);
        $author->setLayout('authorLayout', $this->document)
          ->setTextStyle('authorStyle', $this->document);
        $this->document->addComponent($author);
        // Body.
        $layout = new Document\Layouts\ComponentLayout();
        $layout->setColumnStart(0)->setColumnSpan(5)
          ->setMargin(new Document\Margin(15, 15));
        $this->document->addComponentLayout('bodyLayout', $layout);
        $style = new Document\Styles\ComponentTextStyle();
        $style->setTextAlignment('left')
          ->setFontName('Georgia')
          ->setFontSize(18)
          ->setLineHeight(26)
          ->setTextColor('#000');
        $this->document->addComponentTextStyle('bodyStyle', $style);
        $body->setLayout('bodyLayout', $this->document)
          ->setTextStyle('bodyStyle', $this->document);
        $this->document->addComponent($body);
        break;

    }

    return $this->document;
  }

  /**
   * {@inheritdoc}
   */
  public function assets($entity) {
    $assets = [];
    /** @var EntityDrupalWrapper $wrapper */
    $wrapper = entity_metadata_wrapper('node', $entity);
    // Image field.
    /** @var EntityDrupalWrapper $file */
    $file = $wrapper->__isset('field_image') ? $wrapper->field_image->value() : FALSE;
    if (count($file)) {
      $assets[] = drupal_realpath($file['uri']);
    }
    return $assets;
  }

}

/**
 * Exports a Drupal article as defined by the default install profile.
 */
class ApplenewsExportConfigurableArticle extends ApplenewsExportConfigurable {

  /**
   * {@inheritdoc}
   */
  public function query() {
    return parent::query()
      ->entityCondition('entity_type', 'node')
      ->entityCondition('bundle', ['article'])
      ->propertyCondition('status', NODE_PUBLISHED)
      ->entityOrderBy('entity_id');
  }

  /**
   * {@inheritdoc}
   */
  public function entityTypes() {
    return ['node'];
  }

  /**
   * {@inheritdoc}
   */
  public function bundles() {
    return ['article'];
  }

}
