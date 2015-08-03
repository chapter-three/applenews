<?php

/**
 * @file
 * Destination classes.
 */

use \ChapterThree\AppleNews\Document\Components\Body;
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
   * @param object $value
   *   Value.
   *
   * @return array
   *   A list of \ChapterThree\AppleNews\Document\Base objects.
   */
  public function export(stdClass $value);

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
  abstract public function export(stdClass $value);

  /**
   * Root-only -- can not be a child.
   *
   * @return bool
   *   Is a root.
   */
  public function isRoot() {
    return TRUE;
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
   * Get the value key for a source object.
   *
   * @return string
   *   E.g. sourceobj->export()[$key]
   */
  public function getSourceKey($key) {
    $key = explode('::', $key);
    return $key[count($key) - 1];
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
      'format' => 'markdown',
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
  public function isRoot() {
    return FALSE;
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

    $form['format'] = [
      '#title' => t('Format'),
      '#type' => 'select',
      '#default_value' => $this->settings['format'],
      '#options' => [
        '' => t('(None)'),
        'markdown' => t('Markdown'),
      ],
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function export(stdClass $value) {
    $source = $this->getSource($value, $this->settings['text']);
    $key = $this->getSourceKey($this->settings['text']);
    $value = $source->export()[$key];
    $identifier = isset($array['identifier']) ? $array['identifier'] : NULL;
    $body = new Body(Markdown::convert($value), $identifier);
    $body->setFormat($this->settings['format']);
    return [$body];
  }

}
