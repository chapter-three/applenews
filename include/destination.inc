<?php

/**
 * @file
 * Destination classes.
 */

use \ChapterThree\AppleNews\Document\Components;
use \ChapterThree\AppleNews\Document\GalleryItem;
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
    $options = [];

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
    $components = array();
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

    $components = array();
    foreach ($values as $value) {
      $components = array_merge($components,
        $this->exportConvertHTML($value[$key]));
    }
    return $components;
  }

}

/**
 * Mosaic component.
 */
class ApplenewsDestinationMosaic extends ApplenewsDestination {

  /**
   * {@inheritdoc}
   */
  public function __construct() {
    parent::__construct();
    $this->settings += [
      'items' => NULL,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return t('Mosaic') . ' (' . $this->id . ')';
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {

    $form = parent::getConfigForm($form, $form_state);

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
      $mosaic = new Components\Mosaic($items);
      return [$mosaic];
    }
    return [];
  }

}
