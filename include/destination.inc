<?php

/**
 * @file
 * Destination classes.
 */

use \ChapterThree\AppleNews\Document\Components;
use \ChapterThree\AppleNews\Document\Styles;

/**
 * Base class for a configurable Apple News destination.
 */
abstract class ApplenewsDestination {

  /**
   * Settings for this destination.
   *
   * @var array
   *   Associative array keyed on setting key.
   */
  public $settings = [];

  /**
   * Weight among a set of destinations.
   *
   * @var int
   */
  public $weight = 0;

  /**
   * ID of parent destination among a set of destinations.
   *
   * @var int
   */
  public $pid;

  /**
   * Unique id among a set of destinations.
   *
   * @var int|string
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
  }

  /**
   * Administrative label.
   */
  public function label() {
    if ($this->id === NULL) {
      return '';
    }
    return '(' . $this->id . ')';
  }

  /**
   * Exports to an Apple News Document object.
   *
   * @return array
   *   A list of \ChapterThree\AppleNews\Document\Base objects.
   */
  abstract public function export();

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
   * Validate a source key.
   *
   * @param string $key
   *   Key to search for.
   * @param array|null $options
   *   Internal use.
   *
   * @return bool
   *   Key is valid.
   */
  public function sourceOptionValid($key, $options = NULL) {
    if (!isset($options)) {
      list($options,) = $this->getSourceOptions();
    }
    if (isset($options[$key]) && !is_array($options[$key])) {
      return TRUE;
    }
    foreach (array_filter($options, 'is_array') as $options) {
      if ($this->sourceOptionValid($key, $options)) {
        return TRUE;
      }
    }
    return FALSE;
  }

  /**
   * Generates a configurable source setting element.
   */
  protected function getConfigFormSourceElement(&$form, &$form_state, $key) {
    list($options, $metadata) = $this->getSourceOptions();
    $form[$key] = [
      '#type'  => 'fieldset',
      '#tree'  => TRUE,
      'source' => [
        '#title'         => t('Source'),
        '#type'          => 'select',
        '#options'       => $options,
        '#default_value' => @$this->getSetting($key)['source'],
      ],
    ];
    // Textarea for literal value sources.
    if (!empty($metadata['types']['value'])) {
      $visible_states = [];
      foreach ($metadata['types']['value'] as $source_key) {
        $visible_states[][':input[name="' . $key . '[source]"]']
          = ['value' => $source_key];
      }
      $form[$key]['value'] = [
        '#title'         => t('Value'),
        '#type'          => 'textarea',
        '#default_value' => @$this->getSetting($key)['value'],
        '#states'        => [
          'visible' => $visible_states,
        ]
      ];
    }
  }

  /**
   * List available source options.
   *
   * @param string $ancestors
   *   Internal use.
   * @param string $entity_type
   *   Internal use.
   */
  public function getSourceOptions($ancestors = '', $entity_type = NULL) {
    module_load_include('inc', 'apple_news');

    // Source implementations.
    $sources = apple_news_get_module_apis('source');
    $value_sources = function() use(&$sources) {
      return array_filter($sources, function($info) {
        return $info['type'] == 'value';
      });
    };
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

    $options = [];
    $metadata = [];
    // Top-level-only.
    if (!$ancestors) {
      $options += [
        '' => '- ' . t('none') . ' -',
      ];
      // Each value source.
      foreach ($value_sources() as $source_info) {
        foreach ($source_info['class']::attributes() as $key => $attr_info) {
          $id = implode('::', [
            $ancestors . '::',
            $source_info['module'],
            $source_info['machine_name'],
            $key,
          ]);
          $label = implode(' -- ', [
            $source_info['name'],
            $attr_info,
          ]);
          $options[$id] = $label;
          $metadata['types']['value'][$id] = $id;
        }
      }
    }
    // Can be called recursively.
    if (!isset($entity_type)) {
      $entity_type = $this->export->entityType();
    }
    // Classify so we can order options.
    $opts_atts = $opts_fields = [];
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
          $label = $attr_info['label'];
          list($opts_atts[$label], $m)
            = $this->getSourceOptions($id, $attr_info['type']);
          $metadata += $m;
        }
        // Value source.
        else {
          $label = implode(' -- ', [
            $source_info['name'],
            $attr_info,
          ]);
          $opts_atts[$id] = $label;
          $metadata['types']['entity'][$id] = $id;
        }
      }
    }
    // Each field source that applies to $entity_type.
    $bundles = empty($ancestors) ? $this->export->bundles() : FALSE;
    foreach ($field_types($entity_type, $bundles) as
             $field_name => $field_type
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
          // Nested entity source.
          if (is_array($attr_info)) {
            $label = implode(' -- ', [
              $source_info['name'],
              $field_name,
              $attr_info['label']
            ]);
            list($opts_atts[$label], $m)
              = $this->getSourceOptions($id, $attr_info['type']);
            $metadata += $m;
          }
          // Value source.
          else {
            $label = implode(' -- ', [
              $source_info['name'],
              $field_name,
              $attr_info,
            ]);
            $opts_fields[$id] = $label;
            $metadata['types']['field'][$id] = $id;
          }
        }
      }
    }
    $options += $opts_atts + $opts_fields;
    return [$options, $metadata];
  }

  /**
   * Load a source object.
   *
   * @return ApplenewsSourceInterface
   *   A source object.
   */
  public function getSource(stdClass $value, $key_settings) {
    $implementations = apple_news_get_module_apis('source');

    $key = @$key_settings['source'];
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
      $id = _apple_news_export_id($module, $machine_name);
      if (empty($implementations[$id])) {
        $msg = t('Could not load source %machine_name defined by %module.');
        $var = [
          '%machine_name' => $machine_name,
          '%module' => $module,
        ];
        watchdog('apple_news', $msg, $var, WATCHDOG_ERROR);
        return NULL;
      }
      $key = array_pop($args);
      // @todo this needs to be configurable in the class.
      $reflection = new ReflectionClass($implementations[$id]['class']);
      if ($reflection->isSubclassOf('ApplenewsSourceValue')) {
        // Add literal.
        $args[] = $key_settings[$key];
      }
      else {
        // Add entity.
        $args[] = $value;
      }
      // Instantiate.
      $source = call_user_func_array('apple_news_new_source', $args);
    }

    return $source;
  }

  /**
   * Helper to get a list of values and the configured source key.
   */
  public function getSourceValues(stdClass $value, $source_key) {
    $source = $this->getSource($value, $source_key);
    if (empty($source_key['source'])) {
      return [NULL, []];
    }
    $key = explode('::', $source_key['source']);
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
   * Declare settins and defaults.
   *
   * @return array
   *   Hash of valid keys for getSetting() and setSetting() to their default
   *   value.
   */
  public function settings() {
    return [];
  }

  /**
   * Get a user-configured setting, or default value.
   *
   * @param string $key
   *   Setting key.
   *
   * @return mixed
   *   The set value, a default, or NULL if undefined.
   */
  public function getSetting($key) {
    $defaults = $this->settings();
    if (array_key_exists($key, $defaults)) {
      if (array_key_exists($key, $this->settings)) {
        return $this->settings[$key];
      }
      return $defaults[$key];
    }
    return NULL;
  }

  /**
   * Set a user-configured setting.
   *
   * @param string $key
   *   Setting key.
   * @param mixed $value
   *   Setting value.
   *
   * @return ApplenewsDestination|bool
   *   $this for chaining, or FALSE if $key undefined.
   */
  public function setSetting($key, $value) {
    $defaults = $this->settings();
    if (array_key_exists($key, $defaults)) {
      $this->settings[$key] = $value;
    }
    else {
      return FALSE;
    }
    return $this;
  }

  /**
   * Determines if the configuration is complete.
   *
   * @return bool
   *   If FALSE, then export() will return an empty array.
   */
  public function validateSettings() {
    // No required configuration.
    return TRUE;
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
    foreach (array_keys($this->settings()) as $key) {
      if (isset($form_state['values'][$key])) {
        $this->setSetting($key, $form_state['values'][$key]);
      }
    }
  }

}
