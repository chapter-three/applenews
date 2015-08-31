<?php

/**
 * @file
 * Component Destination classes.
 */

use \ChapterThree\AppleNews\Document\Components;
use \ChapterThree\AppleNews\Document\GalleryItem;
use \ChapterThree\AppleNews\Document\Markdown;
use \ChapterThree\AppleNews\Document\Styles;

/**
 * Base class for component destinations.
 */
abstract class ApplenewsDestinationComponent extends ApplenewsDestination {

  /**
   * Settings for this destination.
   *
   * @var ApplenewsDestinationComponentLayout|string
   *   ComponentLayout object for this component.
   */
  public $layout;

  /**
   * Implements __sleep().
   */
  public function __sleep() {
    return array_merge([
      'layout',
    ], parent::__sleep());
  }

  /**
   * Determines if the configuration is complete.
   *
   * @return bool
   *   If FALSE, then export() will return an empty array.
   */
  public function validateSettings() {
    if ($this->layout && !$this->export->getComponentLayout($this->layout)) {
      return FALSE;
    }
    return parent::validateSettings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $options = [
      '' => '- ' . t('none') . ' -',
    ];
    foreach ($this->export->getComponentLayouts() as $component_layout) {
      $options[$component_layout->id] = $component_layout->label();
    }
    $form['layout'] = [
      '#title' => t('Layout'),
      '#type' => 'select',
      '#options' => $options,
      '#default_value' => $this->layout,
    ];

    $form = parent::getConfigForm($form, $form_state);

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigForm($form, &$form_state) {
    if ($form_state['values']['layout']) {
      $this->layout = $form_state['values']['layout'];
    }
    parent::submitConfigForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function export($value = NULL) {
    /** @var \ChapterThree\AppleNews\Document\Components\Component $component */
    $component = $this->export($value);
    if ($this->layout) {
      $component->setLayout($this->layout);
    }
    return $component;
  }

}

/**
 * Container component.
 */
class ApplenewsDestinationContainer extends ApplenewsDestinationComponent {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Container') . ' ' . parent::label();
  }

  /**
   * {@inheritdoc}
   */
  public function isLeaf() {
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value = NULL) {
    if (empty($value)) {
      return [];
    }

    $container = new Components\Container();
    foreach ($this->children as $component) {
      $container->addComponent($component);
    }
    return $container;
  }

}

/**
 * Body component.
 */
class ApplenewsDestinationBody extends ApplenewsDestinationComponent {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Body') . ' ' . parent::label();
  }

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'text'   => NULL,
      'format' => 'markdown',
    ] + parent::settings();
  }

  /**
   * {@inheritdoc}
   */
  public function validateSettings() {
    foreach (['text'] as $key) {
      if (@$this->getSetting($key) === NULL ||
          !$this->sourceOptionValid($this->getSetting($key)['source'])
      ) {
        return FALSE;
      }
    }
    return parent::validateSettings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    parent::getConfigFormSourceElement($form, $form_state, 'text');
    $form['text']['#title'] = t('Text');
    $form['text']['#required'] = TRUE;

    $form['format'] = [
      '#title'         => t('Format'),
      '#type'          => 'select',
      '#options'       => [
        ''         => t('None'),
        'markdown' => t('Markdown'),
      ],
      '#default_value' => $this->getSetting('format'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value = NULL) {
    if (empty($value)) {
      return [];
    }

    list($key, $values) = $this->getSourceValues($value,
      $this->getSetting('text'));

    if ($this->getSetting('format') == 'markdown') {
      $markdown = new Markdown();
    }

    $components = [];
    foreach ($values as $value) {
      $body = $value[$key];
      if ($this->getSetting('format') == 'markdown') {
        $body = $markdown->convert($body);
      }
      $body = new Components\Body($body);
      if ($this->getSetting('format')) {
        $body->setFormat($this->getSetting('format'));
      }
      $components[] = $body;
    }
    return $components;
  }

}

/**
 * Body with embedded images.
 */
class ApplenewsDestinationBodyPhoto extends ApplenewsDestinationBody {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Body and Photos') . ' (' . $this->id . ')';
  }

  /**
   * Create image object from img element src attribute value.
   */
  public function exportCreateImage($src) {
    return new Components\Photo($this->export->addAsset($src));
  }

  /**
   * Converts an HTML string into a series of Body and image components.
   */
  public function exportConvertHtml($value) {
    $components = [];

    // Convert to markdown, leaving image elements inline.
    $markdown = new Markdown(['img']);
    $markdown = $markdown->convert($value);

    // Concatenate blocks into body component(s), inserting image components
    // after the image-containing block.
    $blocks = [];
    // Each markdown block.
    foreach (explode(Markdown::BLOCK_DELIMITER, $markdown) as $block) {
      // Find img tags.
      $images = [];
      // @todo This regex may be a bit simplistic.
      if (preg_match_all("/<img\s[^>]+>/", $block, $matches)) {
        foreach ($matches as $match) {
          // Get src attribute.
          if (preg_match("/(?<=\ssrc=\")[^\"]+/", $match[0], $src)) {
            $images[] = $src[0];
          }
          // Remove tag.
          $block = substr_replace($block, '',
            strpos($block, $match[0]), strlen($match[0]));
        }
      }
      // Create body component if necessary, then append image components.
      if (count($images)) {
        $body = '';
        if (count($blocks)) {
          $body = implode(Markdown::BLOCK_DELIMITER, $blocks);
          $blocks = [];
        }
        $body .= Markdown::BLOCK_DELIMITER . $block;
        if (preg_match("/\S/", $body)) {
          $body = new Components\Body(trim($body));
          $body->setFormat('markdown');
          $components[] = $body;
        }
        foreach ($images as $src) {
          $components[] = $this->exportCreateImage($src);
        }
      }
      else {
        $blocks[] = $block;
      }
    }
    // Last body component.
    if (count($blocks)) {
      $body = implode(Markdown::BLOCK_DELIMITER, $blocks);
      if (preg_match("/\S/", $body)) {
        $body = new Components\Body(trim($body));
        $body->setFormat('markdown');
        $components[] = $body;
      }
    }

    return $components;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value = NULL) {
    if (empty($value)) {
      return [];
    }

    list($key, $values) = $this->getSourceValues($value,
      $this->getSetting('text'));

    $components = [];
    foreach ($values as $value) {
      $components = array_merge($components,
        $this->exportConvertHTML($value[$key]));
    }
    return $components;
  }

}

/**
 * Mosaic or Gallery component.
 */
class ApplenewsDestinationImages extends ApplenewsDestinationComponent {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Image Series (Gallery or Mosaic)') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'type'    => 'gallery',
      'items'   => NULL,
      'caption' => NULL,
    ] + parent::settings();
  }

  /**
   * {@inheritdoc}
   */
  public function validateSettings() {
    foreach (['items'] as $key) {
      if (@$this->getSetting($key) === NULL ||
          !$this->sourceOptionValid($this->getSetting($key)['source'])
      ) {
        return FALSE;
      }
    }
    return parent::validateSettings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['type'] = [
      '#title'         => t('Type'),
      '#required'      => TRUE,
      '#type'          => 'select',
      '#options'       => [
        'gallery' => t('Gallery'),
        'mosaic'  => t('Mosaic'),
      ],
      '#default_value' => $this->getSetting('type'),
    ];

    parent::getConfigFormSourceElement($form, $form_state, 'items');
    $form['items']['#title'] = t('Items');
    $form['items']['#required'] = TRUE;

    parent::getConfigFormSourceElement($form, $form_state, 'caption');
    $form['caption']['#title'] = t('Caption');
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value = NULL) {
    if (empty($value)) {
      return [];
    }

    list($key, $values) = $this->getSourceValues($value,
      $this->getSetting('items'));
    list($key_caption, $values_caption) = $this->getSourceValues($value,
      $this->getSetting('caption'));

    $items = [];
    foreach ($values as $i => $value) {
      if (empty($value[$key])) {
        continue;
      }
      $url = $this->export->addAsset($value[$key]);
      $item = new GalleryItem($url);
      if (!empty($values_caption[$i][$key_caption])) {
        $item->setCaption($values_caption[$i][$key_caption]);
      }
      $items[] = $item;
    }

    if (count($items)) {
      switch ($this->getSetting('type')) {

        case 'gallery':
          return [new Components\Gallery($items)];

        case 'mosaic':
          return [new Components\Mosaic($items)];

      }
    }

    return [];
  }

}

/**
 * Mosaic component.
 */
class ApplenewsDestinationAd extends ApplenewsDestinationComponent {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Advertisement') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'type'        => 'banner',
      'banner_type' => 'standard',
    ] + parent::settings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['type'] = [
      '#title'         => t('Type'),
      '#required'      => TRUE,
      '#type'          => 'select',
      '#options'       => [
        'banner'           => t('Banner'),
        'medium_rectangle' => t('Medium Rectangle'),
      ],
      '#default_value' => $this->getSetting('type'),
    ];

    $form['banner_type'] = [
      '#title'         => t('Banner Type'),
      '#type'          => 'select',
      '#options'       => [
        'any'           => t('Any'),
        'standard'      => t('Standard'),
        'double_height' => t('Double Height'),
        'large'         => t('Large'),
      ],
      '#default_value' => $this->getSetting('banner_type'),
      '#states'        => array(
        'visible' => array(
          ':input[name="type"]' => array('value' => 'banner'),
        ),
      ),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value = NULL) {

    switch ($this->getSetting('type')) {

      case 'medium_rectangle':
        $ad = new Components\Advertisements\MediumRectangleAdvertisement();
        break;

      case 'banner':
        $ad = new Components\Advertisements\BannerAdvertisement();
        $ad->setBannerType($this->getSetting('banner_type'));
        break;

      default:
        return [];

    }

    return [$ad];
  }

}

/**
 * Image component.
 */
class ApplenewsDestinationImage extends ApplenewsDestinationComponent {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Image') . ' ' . $this->getSetting('type') .
      ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'type'    => 'logo',
      'source'  => NULL,
      'caption' => NULL,
    ] + parent::settings();
  }

  /**
   * {@inheritdoc}
   */
  public function validateSettings() {
    foreach (['source'] as $key) {
      if (@$this->getSetting($key) === NULL ||
          !$this->sourceOptionValid($this->getSetting($key)['source'])
      ) {
        return FALSE;
      }
    }
    return parent::validateSettings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['type'] = [
      '#title'         => t('Type'),
      '#type'          => 'select',
      '#options'       => [
        'logo' => t('Logo')
      ],
      '#default_value' => $this->getSetting('type'),
    ];

    parent::getConfigFormSourceElement($form, $form_state, 'source');
    $form['source']['#title'] = t('Value');
    $form['source']['#required'] = TRUE;

    parent::getConfigFormSourceElement($form, $form_state, 'caption');
    $form['caption']['#title'] = t('Caption');

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value = NULL) {
    if (empty($value)) {
      return [];
    }

    list($key, $values) = $this->getSourceValues($value,
      $this->getSetting('source'));
    list($key_caption, $values_caption) = $this->getSourceValues($value,
      $this->getSetting('caption'));

    $components = [];
    foreach ($values as $i => $value) {
      if (empty($value[$key])) {
        continue;
      }
      $url = $this->export->addAsset($value[$key]);
      switch ($this->getSetting('type')) {
        case 'logo':
          $component = new Components\Logo($url);
          if (!empty($values_caption[$i][$key_caption])) {
            $component->setCaption($values_caption[$i][$key_caption]);
          }
          $components[] = $component;
          break;
      }
    }
    return $components;
  }

}

/**
 * Scalable Image component.
 */
class ApplenewsDestinationScalableImage extends ApplenewsDestinationComponent {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Scalable Image') . ' ' . $this->getSetting('type') .
      ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'type'                 => 'figure',
      'file'                 => NULL,
      'caption'              => NULL,
      'accessibilityCaption' => NULL,
      'explicitContent'      => NULL,
    ] + parent::settings();
  }

  /**
   * {@inheritdoc}
   */
  public function validateSettings() {
    foreach ([
        'caption',
        'accessibilityCaption',
        'explicitContent',
      ] as $key
    ) {
      if (@$this->getSetting($key) === NULL ||
          !$this->sourceOptionValid($this->getSetting($key)['source'])
      ) {
        return FALSE;
      }
    }
    return parent::validateSettings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['type'] = [
      '#title'         => t('Type'),
      '#type'          => 'select',
      '#options'       => [
        'figure'   => t('Figure'),
        'photo'    => t('Photo'),
        'portrait' => t('Portrait'),
      ],
      '#default_value' => $this->getSetting('type'),
      '#description'   => t('The type of scalable image.'),
    ];

    parent::getConfigFormSourceElement($form, $form_state, 'file');
    $form['file']['#title'] = t('Value');

    parent::getConfigFormSourceElement($form, $form_state, 'caption');
    $form['caption']['#title'] = t('Caption');

    parent::getConfigFormSourceElement($form, $form_state,
      'accessibilityCaption');
    $form['accessibilityCaption']['#title'] = t('Accessibility Caption');

    parent::getConfigFormSourceElement($form, $form_state, 'explicitContent');
    $form['explicitContent']['#title'] = t('Explicit Content');

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value = NULL) {
    if (empty($value)) {
      return [];
    }

    list($key, $values) = $this->getSourceValues($value,
      $this->getSetting('file'));
    list($key_caption, $values_caption) = $this->getSourceValues($value,
      $this->getSetting('caption'));
    list($accessibility_caption_key, $accessibility_caption_value)
      = $this->getSourceValues($value,
          $this->getSetting('accessibilityCaption'));
    list($explicit_content_key, $explicit_content_value)
      = $this->getSourceValues($value,
          $this->getSetting('explicitContent'));
    $components = [];
    foreach ($values as $i => $value) {
      if (empty($value[$key])) {
        continue;
      }
      $url = $this->export->addAsset($value[$key]);
      switch ($this->getSetting('type')) {

        case 'photo':
          $component = new Components\Photo($url);
          break;

        case 'portrait':
          $component = new Components\Portrait($url);
          break;

        case 'figure':
          $component = new Components\Figure($url);
          break;

      }
      if (!empty($accessibility_caption_value[$i]['value']) && is_object($accessibility_caption_value[$i]['value'])) {
        $component->setAccessibilityCaption($this->export->addAsset($accessibility_caption_value[$i]['value']));
      }
      if (!empty($explicit_content_value[$i]['value'])) {
        $component->setExplicitContent(TRUE);
      }
      if (!empty($values_caption[$i][$key_caption])) {
        $component->setCaption($values_caption[$i][$key_caption]);
      }
      $components[] = $component;
    }
    return $components;
  }

}

/**
 * Text component.
 */
class ApplenewsDestinationText extends ApplenewsDestinationComponent {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Text') . ' ' . $this->getSetting('component') .
      ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'text'      => NULL,
      'component' => 'author',
      'heading'   => 'heading',
      'format'    => 'markdown',
    ] + parent::settings();
  }

  /**
   * {@inheritdoc}
   */
  public function validateSettings() {
    foreach (['text'] as $key) {
      if (@$this->getSetting($key) === NULL ||
          !$this->sourceOptionValid($this->getSetting($key)['source'])
      ) {
        return FALSE;
      }
    }
    return parent::validateSettings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    parent::getConfigFormSourceElement($form, $form_state, 'text');
    $form['text']['#title'] = t('Text');
    $form['text']['#required'] = TRUE;

    $form['component'] = [
      '#title'         => t('Component'),
      '#required'      => TRUE,
      '#type'          => 'select',
      '#options'       => [
        'author'       => t('Author'),
        'body'         => t('Body'),
        'byline'       => t('Byline'),
        'caption'      => t('Caption'),
        'heading'      => t('Heading'),
        'illustrator'  => t('Illustrator'),
        'intro'        => t('Intro'),
        'photographer' => t('Photographer'),
        'pullquote'    => t('Pullquote'),
        'quote'        => t('Quote'),
        'title'        => t('Title'),
      ],
      '#default_value' => $this->getSetting('component'),
    ];

    $form['heading'] = [
      '#title'         => t('Heading'),
      '#type'          => 'select',
      '#options'       => [
        'heading'  => t('Heading'),
        'heading1' => t('H1'),
        'heading2' => t('H2'),
        'heading3' => t('H3'),
        'heading4' => t('H4'),
        'heading5' => t('H5'),
        'heading6' => t('H6'),
      ],
      '#default_value' => $this->getSetting('heading'),
      '#states'        => [
        'visible' => [
          ':input[name="component"]' => ['value' => 'heading'],
        ],
      ],
    ];

    $form['format'] = [
      '#title'         => t('Format'),
      '#type'          => 'select',
      '#options'       => [
        ''         => t('None'),
        'markdown' => t('Markdown'),
      ],
      '#default_value' => $this->getSetting('format'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value = NULL) {
    if (empty($value)) {
      return [];
    }

    list($key, $values) = $this->getSourceValues($value,
      $this->getSetting('text'));

    $markdown = new Markdown();

    $components = [];
    foreach ($values as $value) {
      switch ($this->getSetting('component')) {

        case 'author':
          if ($this->getSetting('format')) {
            $component = new Components\Author($markdown->convert($value[$key]));
            $component->setFormat($this->getSetting('format'));
          }
          else {
            $component = new Components\Author($value[$key]);
          }
          $components[] = $component;
          break;

        case 'body':
          if ($this->getSetting('format')) {
            $component = new Components\Body($markdown->convert($value[$key]));
            $component->setFormat($this->getSetting('format'));
          }
          else {
            $component = new Components\Body($value[$key]);
          }
          break;

        case 'byline':
          if ($this->getSetting('format')) {
            $component = new Components\Byline($markdown->convert($value[$key]));
            $component->setFormat($this->getSetting('format'));
          }
          else {
            $component = new Components\Byline($value[$key]);
          }
          break;

        case 'caption':
          if ($this->getSetting('format')) {
            $component = new Components\Caption($markdown->convert($value[$key]));
            $component->setFormat($this->getSetting('format'));
          }
          else {
            $component = new Components\Caption($value[$key]);
          }
          break;

        case 'heading':
          if ($this->getSetting('format')) {
            $component = new Components\Heading($markdown->convert($value[$key]));
            $component->setFormat($this->getSetting('format'));
          }
          else {
            $component = new Components\Heading($value[$key]);
          }
          $component->setRole($this->getSetting('heading'));
          break;

        case 'illustrator':
          if ($this->getSetting('format')) {
            $component = new Components\Illustrator($markdown->convert($value[$key]));
            $component->setFormat($this->getSetting('format'));
          }
          else {
            $component = new Components\Illustrator($value[$key]);
          }
          break;

        case 'intro':
          if ($this->getSetting('format')) {
            $component = new Components\Intro($markdown->convert($value[$key]));
            $component->setFormat($this->getSetting('format'));
          }
          else {
            $component = new Components\Intro($value[$key]);
          }
          break;

        case 'photographer':
          if ($this->getSetting('format')) {
            $component = new Components\Photographer($markdown->convert($value[$key]));
            $component->setFormat($this->getSetting('format'));
          }
          else {
            $component = new Components\Photographer($value[$key]);
          }
          break;

        case 'pullquote':
          if ($this->getSetting('format')) {
            $component = new Components\Pullquote($markdown->convert($value[$key]));
            $component->setFormat($this->getSetting('format'));
          }
          else {
            $component = new Components\Pullquote($value[$key]);
          }
          break;

        case 'quote':
          if ($this->getSetting('format')) {
            $component = new Components\Quote($markdown->convert($value[$key]));
            $component->setFormat($this->getSetting('format'));
          }
          else {
            $component = new Components\Quote($value[$key]);
          }
          break;

        case 'title':
        default:
          if ($this->getSetting('format')) {
            $component = new Components\Title($markdown->convert($value[$key]));
            $component->setFormat($this->getSetting('format'));
          }
          else {
            $component = new Components\Title($value[$key]);
          }
          break;

      }
      $components[] = $component;

    }
    return $components;
  }

}

/**
 * Divider component.
 */
class ApplenewsDestinationDivider extends ApplenewsDestinationComponent {

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Divider') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function settings() {
    return [
      'color' => '#000',
      'width' => 1,
      'style' => 'solid',
    ] + parent::settings();
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['color'] = [
      '#title' => t('Color'),
      '#type' => 'textfield',
      '#default_value' => $this->getSetting('color'),
      '#description'   => t('The stroke color, defined as a 3- to 8-character RGBA hexadecimal string; e.g., #000 for black or #FF00007F for red with an alpha (opacity) of 50%. Defaults to #000 (black).')
    ];

    $form['width'] = [
      '#title' => t('Width'),
      '#type' => 'textfield',
      '#default_value' => $this->getSetting('width'),
      '#description'   => t('The width of the stroke line. Can be either an integer value in points, or a string (10pt). Defaults to 1 point.'),
    ];

    $form['style'] = [
      '#title' => t('Style'),
      '#type' => 'select',
      '#options' => [
        'solid' => t('Solid'),
        'dashed' => t('Dashed'),
        'dotted' => t('Dotted'),
      ],
      '#default_value' => $this->getSetting('style'),
      '#description'   => t('The stroke properties that apply to the horizontal line.'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value = NULL) {
    $divider = new Components\Divider();
    if ($this->getSetting('color')) {
      $stroke = new Styles\StrokeStyle();
      $stroke->setColor($this->getSetting('color'));
      $stroke->setWidth((int) $this->getSetting('width'));
      $stroke->setStyle($this->getSetting('style'));
      $divider->setStroke($stroke);
    }
    return [$divider];
  }

}
