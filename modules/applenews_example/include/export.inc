<?php

/**
 * @file
 * Export classes.
 */

use \ChapterThree\AppleNewsAPI\Document;
use \ChapterThree\AppleNewsAPI\Document\Anchor;
use \ChapterThree\AppleNewsAPI\Document\Components\Body;
use \ChapterThree\AppleNewsAPI\Document\Components\Image;
use \ChapterThree\AppleNewsAPI\Document\Components\ScalableImage;
use \ChapterThree\AppleNewsAPI\Document\Components\Pullquote;
use \ChapterThree\AppleNewsAPI\Document\Components\ComponentNested;

/**
 * A configurable node export.
 */
class ApplenewsexampleExportNode extends ApplenewsExportNode {

  /**
   * {@inheritdoc}
   */
  public function __construct($module, $machine_name, array $arguments) {
    parent::__construct($module, $machine_name, $arguments);

    // Apply to articles if exists.
    $entities_info = entity_get_info('node');
    if (isset($entities_info['bundles']['article'])) {
      $this->bundles = ['article'];
    }

    // Metadata.
    /** @var ApplenewsDestinationMetadata $metadata */
    $metadata = applenews_new_destination('applenews', 'metadata');
    $metadata
      ->setSetting('date_published', [
        'source' => '::::applenews::node::created',
        'value' => '',
      ])
      ->setSetting('date_created', [
        'source' => '::::applenews::node::created',
        'value' => '',
      ])
      ->setSetting('date_modified', [
        'source' => '::::applenews::node::changed',
        'value' => '',
      ]);
    $this->setMetadata($metadata);

    // Header.
    /** @var ApplenewsDestinationText $component */
    $component = applenews_new_destination('applenews_example', 'header_image');
    $component->weight = '0';
    $component->title = 'Header Image';
    $component
      ->setSetting('file', [
        'source' => '::::applenews::file::node::field_image::uri',
        'value' => '',
      ]);
    $this->addComponent($component);

    // Title/Credits.
    /** @var ApplenewsexampleDestinationHeaderGradContainer $component */
    $component = applenews_new_destination('applenews_example', 'header_container');
    $component->weight = '1';
    $component->title = 'Title/Credits';
    $component
      ->setSetting('title', [
        'source' => '::::applenews::node::label',
        'value' => '',
      ])
      ->setSetting('author', [
        'source' => '::::applenews::node::author::::applenews::user::label',
        'value' => '',
      ]);
    $this->addComponent($component);

    // Ad.
    /** @var ApplenewsexampleDestinationAd $component */
    $component = applenews_new_destination('applenews_example', 'ad');
    $component->weight = '2';
    $component->title = 'Ad';
    $this->addComponent($component);

    // Body.
    /** @var ApplenewsexampleDestinationBody $component */
    $component = applenews_new_destination('applenews_example', 'body');
    $component->weight = '3';
    $component->title = 'Body';
    $component
      ->setSetting('text', [
        'source' => '::::applenews::field::node::body::value_sanitized',
        'value' => '',
      ])
      ->setSetting('format', 'markdown');
    $this->addComponent($component);

    // Pull Quote.
    /** @var ApplenewsexampleDestinationPullQuote $component */
    $component = applenews_new_destination('applenews_example', 'pullquote');
    $component->weight = '4';
    $component->title = 'Pull Quote';
    $this->addComponent($component);

    // Gallery.
    /** @var ApplenewsexampleDestinationImages $component */
    $component = applenews_new_destination('applenews_example', 'images');
    $component->weight = '5';
    $component->title = 'Gallery';
    $this->addComponent($component);
    $component
      ->setSetting('type', 'gallery');
  }

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'layout_alignment' => 'left',
    ] + parent::settings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm(array $values = []) {
    $form = parent::getConfigForm($values);

    // Hide parent layout elements.
    $tab = &$form['additional_settings']['layouts_tab'];
    $tab['layout']['#access'] = FALSE;
    $tab['component_layouts']['#access'] = FALSE;

    $tab['layout_alignment'] = [
      '#type'          => 'select',
      '#title'         => t('Alignment'),
      '#required'      => TRUE,
      '#default_value' => $this->getSetting('layout_alignment'),
      '#options'       => [
        'left'   => t('Left'),
        'center' => t('Center'),
        'right'  => t('Right'),
      ],
    ];

    return $form;
  }

  /**
   * Set any relations between components.
   */
  protected function associateComponents() {

    $components = $this->document->getComponents();
    for ($i = 0; $i < count($components); $i++) {

      /** @var \ChapterThree\AppleNewsAPI\Document\Components\Component $comp */
      $comp = $components[$i];

      // Associate with previous component.
      if ($i) {

        $prev = $components[$i - 1];

        // Previous is a body component.
        /** @var \ChapterThree\AppleNewsAPI\Document\Components\Body $prev */
        if ($prev instanceof Body) {

          // Anchor to previous component if not anchor-ed already.
          if (!$comp->getAnchor()) {
            // One of:
            $anchor = $comp instanceof Image ||
              $comp instanceof ScalableImage ||
              $comp instanceof Pullquote;
            $range_start = 0;
            // Or contains pullquote.
            if (!$anchor && $comp instanceof ComponentNested) {
              $class_pullquote = '\ChapterThree\AppleNews\Document\Components\Pullquote';
              /** @var \ChapterThree\AppleNewsAPI\Document\Components\ComponentNested $comp */
              if ($comp->hasComponentType($class_pullquote)) {
                $anchor = TRUE;
                $range_start = floor(strlen($prev->getText()) / 2);
              }
            }
            if ($anchor) {
              if (!$id = $prev->getIdentifier()) {
                $id = $this->generateIdentifier();
                $prev->setIdentifier($id);
              }
              $anchor = new Anchor('bottom');
              $anchor->setOriginAnchorPosition('bottom')
                ->setTargetComponentIdentifier($id)
                ->setRangeStart($range_start)
                ->setRangeLength(1);
              $comp->setAnchor($anchor);
            }
          }
        }
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function exportDocument() {
    parent::exportDocument();

    $layout = $this->document->getLayout();
    switch ($this->getSetting('layout_alignment')) {

      case 'left':
      case 'right':
        $layout->setWidth(1024)->setColumns(7);
        break;

      case 'center':
        $layout->setWidth(768)->setColumns(9);
        break;

    }
    $layout->setMargin(75)->setGutter(20);
    $this->associateComponents();
    return $this->document;
  }

}
