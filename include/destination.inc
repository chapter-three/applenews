<?php

/**
 * @file
 * Destination classes.
 */

use \ChapterThree\AppleNews\Document\Components;
use \ChapterThree\AppleNews\Document\GalleryItem;
use \ChapterThree\AppleNews\Document\Markdown;
use \ChapterThree\AppleNews\Document\Styles;

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
   * @return array
   *   A list of \ChapterThree\AppleNews\Document\Base objects.
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
  public $settings;

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
   * Child components.
   *
   * @var array
   *   List of \ChapterThree\AppleNews\Document objects.
   */
  public $children;

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
    return FALSE;
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
   * If isLeaf() returns FALSE, this should set any child components.
   */
  public function setChildren($children) {
    $this->children = $children;
  }

  /**
   * List available source options.
   *
   * @param string $ancestors
   *   Internal use.
   * @param array $entity_types
   *   Internal use.
   */
  public function getSourceOptions($ancestors = '', array $entity_types = NULL) {
    $options = $ancestors ? [] : ['' => '- ' . t('none') . ' -'];

    // Source implementations.
    $sources = apple_news_get_module_apis()['sources'];
    $entity_sources = function($entity_type) use(&$sources) {
      return array_filter($sources, function($info) use($entity_type) {
        return $info['type'] == 'entity' &&
          in_array($entity_type, $info['class']::types());
      });
    };
    $field_sources = function($field_type) use(&$sources) {
      return array_filter($sources, function($info) use($field_type) {
        return $info['type'] == 'field' &&
          in_array($field_type, $info['class']::types());
      });
    };

    // Array of field type keyed on field name.
    $field_types = function($entity_type, $bundles = FALSE) {
      $field_info = field_info_instances($entity_type);
      if (is_array($bundles)) {
        foreach (array_keys($field_info) as $bundle) {
          if (!in_array($bundle, $bundles)) {
            unset($field_info[$bundle]);
          }
          else {
            $field_info[$bundle] = array_map(function($info) {
              $info = field_info_field($info['field_name']);
              return $info['type'];
            }, $field_info[$bundle]);
          }
        }
      }
      $field_names = [];
      foreach ($field_info as $fields) {
        $field_names += $fields;
      }
      return $field_names;
    };

    // Can be called recursively.
    if (!isset($entity_types)) {
      $entity_types = $this->export->entityTypes();
    }
    foreach ($entity_types as $entity_type) {
      // Classify so we can order options.
      $opts_refs = $opts_atts = $opts_fields = [];
      // Each entity source that applies to $entity_type.
      foreach ($entity_sources($entity_type) as $source_info) {
        // Entity attributes.
        foreach ($source_info['class']::attributes() as $key => $attr_info) {
          $id = implode('::', [
            $ancestors . '::',
            $source_info['module'],
            $source_info['machine_name'],
            $key,
          ]);
          // Nested entity source.
          if (is_array($attr_info)) {
            $opts_refs[$attr_info['label']] = $this->getSourceOptions($id,
              [$attr_info['type']]);
          }
          // Value source.
          else {
            $opts_atts[$id] = "${source_info['name']} -- ${attr_info}";
          }
        }
      }
      // Each field source that applies to $entity_type.
      $bundles = empty($ancestors) ? $this->export->bundles() : FALSE;
      foreach ($field_types($entity_type, $bundles) as $field_name => $field_type
      ) {
        // Each field source that applies to $field_type.
        foreach ($field_sources($field_type) as $source_info) {
          // Field attributes.
          foreach ($source_info['class']::attributes() as $key => $attr_info) {
            $id = implode('::', [
              $ancestors . '::',
              $source_info['module'],
              $source_info['machine_name'],
              $entity_type,
              $field_name,
              $key,
            ]);
            $opts_fields[$id] = "${source_info['name']} -- ${field_name} -- ${attr_info}";
          }
        }
      }
      $options += $opts_atts + $opts_fields + $opts_refs;
    }
    return $options;
  }

  /**
   * Load a source object.
   *
   * @return ApplenewsSourceInterface
   *   A source object.
   */
  public function getSource(stdClass $value, $key) {
    $implementations = apple_news_get_module_apis()['sources'];

    if (empty($key)) {
      return NULL;
    }

    // Parse key.
    // @see self::getSourceOptions()
    // Source classes may have a parent -- delineated by '::::'.
    $hierarchy = explode('::::', ltrim($key, '::::'));
    // Data needed to instantiate each object -- delineated by '::'.
    while ($hierarchy) {
      $args = explode('::', array_shift($hierarchy));
      // Verify implementation still defined.
      $module = $args[0];
      $machine_name = $args[1];
      $id = apple_news_export_id($module, $machine_name);
      if (empty($implementations[$id])) {
        $msg = t('Could not load source %machine_name defined by %module.');
        $var = [
          '%machine_name' => $machine_name,
          '%module' => $module,
        ];
        watchdog('apple_news', $msg, $var, WATCHDOG_ERROR);
        return NULL;
      }
      // Don't need key.
      array_pop($args);
      // Add entity.
      $args[] = $value;
      // Instantiate.
      $source = call_user_func_array('apple_news_get_source', $args);
    }

    return $source;
  }

  /**
   * Helper to get a list of values and the configured source key.
   */
  public function getSourceValues(stdClass $value, $source_key) {
    $source = $this->getSource($value, $source_key);
    $key = explode('::', $source_key);
    $key = array_pop($key);
    if (empty($source)) {
      return [$key, []];
    }
    $values = $source->export();
    // Handle single/multiple values similarly, as a list of values.
    if (!isset($values[0])) {
      $values = [$values];
    }
    return [$key, $values];
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
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['text'] = [
      '#title' => t('Text'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['text'],
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['text']);
    $components = [];
    foreach ($values as $value) {
      $body = new Components\Body(Markdown::convert($value[$key]));
      $body->setFormat('markdown');
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

    $value = filter_xss($value, [
      // Handled by ChapterThree\AppleNews\Document\Markdown.
      'a', 'i', 'em', 'b', 'strong',
      'p', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6',
      'ul', 'ol', 'li', 'hr',
      // Handled by this destination.
      'img',
    ]);

    $reader = new \XMLReader();
    if (!$reader->XML("<html>${value}</html>")) {
      $msg = t('Could not parse content as HTML.');
      $var = [];
      watchdog('apple_news', $msg, $var, WATCHDOG_ERROR);
      return $components;
    }

    // When img element found, create Body object with any preceding content
    // then add an image object.
    $body = '';
    // To keep Body markup valid, move nested images outside any containing
    // elements.
    $images = [];
    $add_components = function($src = NULL) use(&$body, &$images, &$components) {
      // If $body contains markup, move into Body component.
      if (!empty($body)) {
        $body = new Components\Body(Markdown::convert($body));
        $body->setFormat('markdown');
        $components[] = $body;
        $body = '';
      }
      // Append any nested img from preceding markup.
      if (!empty($images)) {
        while ($image = array_shift($images)) {
          $components[] = $this->exportCreateImage($image);
        }
      }
      // Add image.
      if (!empty($src)) {
        $components[] = $this->exportCreateImage($src);
      }
    };

    // Main event loop.
    $stack = [];
    while ($reader->read()) {

      switch ($reader->nodeType) {

        case \XMLREADER::ELEMENT:
          switch ($reader->localName) {

            case 'i':
            case 'em':
            case 'b':
            case 'strong':
            case 'p':
            case 'h1':
            case 'h2':
            case 'h3':
            case 'h4':
            case 'h5':
            case 'h6':
            case 'ul':
            case 'ol':
            case 'li':
              array_push($stack, $reader->localName);
              $body .= '<' . $reader->localName . '>';
              break;

            case 'a':
              array_push($stack, $reader->localName);
              $reader->moveToAttribute('href');
              if ($reader->value) {
                $body .= '<a href="' . $reader->value . '">';
              }
              break;

            case 'hr':
              $body .= '<' . $reader->localName . '>';
              break;

            case 'img':
              $reader->moveToAttribute('src');
              if ($reader->value) {
                // If contained inside other element, move outside ancestors.
                if (count($stack)) {
                  array_push($images, $reader->value);
                }
                else {
                  $add_components($reader->value);
                }
              }
              break;

          }
          break;

        case \XMLReader::END_ELEMENT:
          switch ($reader->localName) {

            case 'i':
            case 'em':
            case 'b':
            case 'strong':
            case 'p':
            case 'h1':
            case 'h2':
            case 'h3':
            case 'h4':
            case 'h5':
            case 'h6':
            case 'ul':
            case 'ol':
            case 'li':
            case 'a':
              $expected = array_pop($stack);
              if ($reader->localName != $expected) {
                $msg = t('Found %f element but expected %e.');
                $var = ['%f' => $reader->localName, '%e' => $expected];
                watchdog('apple_news', $msg, $var, WATCHDOG_ERROR);
                return $components;
              }
              $body .= '</' . $reader->localName . '>';
              break;

          }
          break;

        case \XMLReader::TEXT:
          $body .= $reader->value;
          break;

      }

      if (!empty($images) && empty($stack)) {
        $add_components();
      }
    }

    $add_components();

    return $components;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['text']);

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
class ApplenewsDestinationImages extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'type' => 'gallery',
      'items' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Image Series (Gallery or Mosaic)') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['type'] = [
      '#title' => t('Type'),
      '#type' => 'select',
      '#options' => [
        'gallery' => t('Gallery'),
        'mosaic' => t('Mosaic'),
      ],
      '#default_value' => $this->settings['type'],
    ];

    $form['items'] = [
      '#title' => t('Items'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['items'],
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['items']);

    $items = [];
    foreach ($values as $i => $value) {
      if (empty($value[$key])) {
        continue;
      }
      $file = $value[$key];
      if (is_object($file)) {
        $item = new GalleryItem($this->export->addAsset($file->uri));
        $item->setCaption(empty($file->title) ? (empty($file->alt) ? '' : $file->alt) : $file->title);
      }
      else {
        $item = new GalleryItem($this->export->addAsset($file));
      }
      $items[] = $item;
    }

    if (count($items)) {
      switch ($this->settings['type']) {

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
class ApplenewsDestinationAd extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'type' => 'banner',
      'banner_type' => 'standard',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Advertisement') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['type'] = [
      '#title' => t('Type'),
      '#type' => 'select',
      '#options' => [
        'banner' => t('Banner'),
        'medium_rectangle' => t('Medium Rectangle'),
      ],
      '#default_value' => $this->settings['type'],
    ];

    $form['banner_type'] = [
      '#title' => t('Banner Type'),
      '#type' => 'select',
      '#options' => [
        'any' => t('Any'),
        'standard' => t('Standard'),
        'double_height' => t('Double Height'),
        'large' => t('Large'),
      ],
      '#default_value' => $this->settings['banner_type'],
      '#states' => array(
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
  public function export($value) {

    switch ($this->settings['type']) {

      case 'medium_rectangle':
        $ad = new Components\Advertisements\MediumRectangleAdvertisement();
        break;

      case 'banner':
      default:
        $ad = new Components\Advertisements\BannerAdvertisement();
        $ad->setBannerType($this->settings['banner_type']);
        break;

    }

    return [$ad];
  }

}

/**
 * Image component.
 */
class ApplenewsDestinationImage extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'type' => NULL,
      'source' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Image') . ' (' . (!empty($this->settings['type']) ? $this->settings['type'] : $this->id) . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['type'] = [
      '#title' => t('Type'),
      '#type' => 'select',
      '#options' => [
        'Logo' => t('Logo')
      ],
      '#default_value' => !empty($this->settings['type']) ? $this->settings['type'] : 'Logo',
    ];

    $form['source'] = [
      '#title' => t('Value'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['source'],
      '#description' => t('<strong>Important:</strong> Map this field to the File field. Images should be high-resolution so they can be smoothly scaled down. Supported image types include JPEG (with .jpg or .jpeg extension), PNG, and GIF.'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['source']);
    $components = [];
    foreach ($values as $value) {
      $file = $value[$key];
      if (is_object($file)) {
        switch ($this->settings['type']) {
          case 'Logo' :
          default :
            $component = new Components\Logo($this->export->addAsset($file->uri));
            $component->setCaption(empty($file->title) ? (empty($file->alt) ? '' : $file->alt) : $file->title);
            $components[] = $component;
            break;
        }
      }
    }
    return $components;
  }

}

/**
 * Scalable Image component.
 */
class ApplenewsDestinationScalableImage extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'type' => NULL,
      'file' => NULL,
      'accessibilityCaption' => NULL,
      'explicitContent' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Scalable Image') . ' (' . (!empty($this->settings['type']) ? $this->settings['type'] : $this->id) . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['type'] = [
      '#title' => t('Type'),
      '#type' => 'select',
      '#options' => [
        'Figure' => t('Figure'),
        'Photo' => t('Photo'),
        'Portrait' => t('Portrait'),
      ],
      '#default_value' => !empty($this->settings['type']) ? $this->settings['type'] : 'Figure',
    ];

    $form['file'] = [
      '#title' => t('Value'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['file'],
      '#description' => t('<strong>Important:</strong> Map this field to the File field. Images should be high-resolution so they can be smoothly scaled down. Supported image types include JPEG (with .jpg or .jpeg extension), PNG, and GIF.'),
    ];

    $form['accessibilityCaption'] = [
      '#title' => t('Accessibility Caption'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['accessibilityCaption'],
      '#description' => t('Optional caption text describing the contents of the image. Note that this property differs from caption although the caption may be displayed to readers, the accessiblity caption is used for VoiceOver only.'),
    ];

    $form['explicitContent'] = [
      '#title' => t('Explicit Content'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['explicitContent'],
      '#description' => t('This property indicates that the audio track may contain explicit/graphic content. Default value is False'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {
    list($key, $values) = $this->getSourceValues($value,
      $this->settings['file']);
    list($accessibilityCaption_key, $accessibilityCaption_value) = $this->getSourceValues($value,
      $this->settings['accessibilityCaption']);
    list($explicitContent_key, $explicitContent_value) = $this->getSourceValues($value,
      $this->settings['explicitContent']);
    $components = [];
    foreach ($values as $value) {
      $file = $value[$key];
      if (is_object($file)) {
        switch ($this->settings['type']) {
          case 'Photo' :
            $component = new Components\Photo($this->export->addAsset($file->uri));
            break;

          case 'Portrait' :
            $component = new Components\Portrait($this->export->addAsset($file->uri));
            break;

          case 'Figure' :
          default :
            $component = new Components\Figure($this->export->addAsset($file->uri));
            break;
        }
        if (!empty($accessibilityCaption_value[0]['value']) && is_object($accessibilityCaption_value[0]['value'])) {
          $component->setAccessibilityCaption($this->export->addAsset($accessibilityCaption_value[0]['value']));
        }
        if (!empty($explicitContent_value[0]['value'])) {
          $component->setExplicitContent(TRUE);
        }
        $component->setCaption(empty($file->title) ? (empty($file->alt) ? '' : $file->alt) : $file->title);
        $components[] = $component;
      }
    }
    return $components;
  }

}

/**
 * Text component.
 */
class ApplenewsDestinationText extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'source' => '',
      'format' => NULL,
      'heading' => NULL,
      'component' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Text') . ' (' . (!empty($this->settings['component']) ? $this->settings['component'] : $this->id) . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['source'] = [
      '#title' => t('Value'),
      '#type' => 'select',
      '#options' => $this->getSourceOptions(),
      '#default_value' => $this->settings['source'],
    ];

    $form['component'] = [
      '#title' => t('Component'),
      '#type' => 'select',
      '#options' => [
        'Author' => t('Author'),
        'Body' => t('Body'),
        'Byline' => t('Byline'),
        'Caption' => t('Caption'),
        'Heading' => t('Heading'),
        'Illustrator' => t('Illustrator'),
        'Intro' => t('Intro'),
        'Photographer' => t('Photographer'),
        'Pullquote' => t('Pullquote'),
        'Quote' => t('Quote'),
        'Title' => t('Title'),
      ],
      '#default_value' => !empty($this->settings['component']) ? $this->settings['component'] : 'Title',
    ];

    $form['heading'] = [
      '#title' => t('Heading'),
      '#type' => 'select',
      '#options' => [
        'heading' => t('Heading'),
        'heading1' => t('H1'),
        'heading2' => t('H2'),
        'heading3' => t('H3'),
        'heading4' => t('H4'),
        'heading5' => t('H5'),
        'heading6' => t('H6'),
      ],
      '#default_value' => $this->settings['heading'],
      '#states' => [
        'visible' => [
          ':input[name="component"]' => ['value' => 'Heading'],
        ],
      ],
    ];

    $form['format'] = [
      '#title' => t('Format'),
      '#type' => 'select',
      '#options' => [
        '' => t('None'),
        'markdown' => t('Markdown'),
      ],
      '#default_value' => $this->settings['format'],
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {

    list($key, $values) = $this->getSourceValues($value,
      $this->settings['source']);

    $components = [];
    foreach ($values as $value) {
      switch ($this->settings['component']) {

        case 'Author':
          if (!empty($this->settings['format'])) {
            $component = new Components\Author(Markdown::convert($value[$key]));
            $component->setFormat($this->settings['format']);
          }
          else {
            $component = new Components\Author($value[$key]);
          }
          $components[] = $component;
          break;

        case 'Body':
          if (!empty($this->settings['format'])) {
            $component = new Components\Body(Markdown::convert($value[$key]));
            $component->setFormat($this->settings['format']);
          }
          else {
            $component = new Components\Body($value[$key]);
          }
          break;

        case 'Byline':
          if (!empty($this->settings['format'])) {
            $component = new Components\Byline(Markdown::convert($value[$key]));
            $component->setFormat($this->settings['format']);
          }
          else {
            $component = new Components\Byline($value[$key]);
          }
          break;

        case 'Caption':
          if (!empty($this->settings['format'])) {
            $component = new Components\Caption(Markdown::convert($value[$key]));
            $component->setFormat($this->settings['format']);
          }
          else {
            $component = new Components\Caption($value[$key]);
          }
          break;

        case 'Heading':
          if (!empty($this->settings['format'])) {
            $component = new Components\Heading(Markdown::convert($value[$key]));
            $component->setFormat($this->settings['format']);
          }
          else {
            $component = new Components\Heading($value[$key]);
          }
          $component->setRole($this->settings['heading']);
          break;

        case 'Illustrator':
          if (!empty($this->settings['format'])) {
            $component = new Components\Illustrator(Markdown::convert($value[$key]));
            $component->setFormat($this->settings['format']);
          }
          else {
            $component = new Components\Illustrator($value[$key]);
          }
          break;

        case 'Intro':
          if (!empty($this->settings['format'])) {
            $component = new Components\Intro(Markdown::convert($value[$key]));
            $component->setFormat($this->settings['format']);
          }
          else {
            $component = new Components\Intro($value[$key]);
          }
          break;

        case 'Photographer':
          if (!empty($this->settings['format'])) {
            $component = new Components\Photographer(Markdown::convert($value[$key]));
            $component->setFormat($this->settings['format']);
          }
          else {
            $component = new Components\Photographer($value[$key]);
          }
          break;

        case 'Pullquote':
          if (!empty($this->settings['format'])) {
            $component = new Components\Pullquote(Markdown::convert($value[$key]));
            $component->setFormat($this->settings['format']);
          }
          else {
            $component = new Components\Pullquote($value[$key]);
          }
          break;

        case 'Quote':
          if (!empty($this->settings['format'])) {
            $component = new Components\Quote(Markdown::convert($value[$key]));
            $component->setFormat($this->settings['format']);
          }
          else {
            $component = new Components\Quote($value[$key]);
          }
          break;

        case 'Title':
        default:
          if (!empty($this->settings['format'])) {
            $component = new Components\Title(Markdown::convert($value[$key]));
            $component->setFormat($this->settings['format']);
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
class ApplenewsDestinationDivider extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'color' => '#000',
      'width' => 1,
      'style' => 'solid',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Divider') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

    $form['color'] = [
      '#title' => t('Color'),
      '#type' => 'textfield',
      '#default_value' => !empty($this->settings['color']) ? $this->settings['color'] : '#000',
      '#description' => t('The stroke color, defined as a 3- to 8-character RGBA hexadecimal string; e.g., #000 for black or #FF00007F for red with an alpha (opacity) of 50%. Defaults to #000 (black).'),
    ];

    $form['width'] = [
      '#title' => t('Width'),
      '#type' => 'textfield',
      '#default_value' => !empty($this->settings['width']) ? $this->settings['width'] : 1,
      '#description' => t('The width of the stroke line. Can be either an integer value in points. Defaults to 1 point.')
    ];

    $form['style'] = [
      '#title' => t('Style'),
      '#type' => 'select',
      '#options' => [
        'solid' => t('Solid'),
        'dashed' => t('Dashed'),
        'dotted' => t('Dotted'),
      ],
      '#default_value' => !empty($this->settings['style']) ? $this->settings['style'] : 'solid',
      '#description' => t('Defines the style of the stroke. Available values: <strong>solid</strong> (Default), <strong>dashed</strong> and <strong>dotted</strong>')
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export($value) {
    $divider = new Components\Divider();
    if (!empty($this->settings['color'])) {
      $stroke = new Styles\StrokeStyle();
      $stroke->setColor($this->settings['color']);
      $stroke->setWidth($this->settings['width']);
      $stroke->setStyle($this->settings['style']);
      $divider->setStroke($stroke);
    }
    return [$divider];
  }

}
