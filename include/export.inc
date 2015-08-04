<?php

/**
 * @file
 * Export classes.
 */

use ChapterThree\AppleNews\Document;

/**
 * Base class for exporting Drupal entities.
 */
abstract class ApplenewsExport {

  // @see __construct().
  protected $module;
  protected $machineName;
  protected $arguments;

  /**
   * Export is enabled.
   *
   * @var bool
   */
  public $enabled = FALSE;

  /**
   * User-configurable settings, defined in $this->getConfigForm().
   *
   * @var array
   */
  public $settings = [];

  /**
   * Apple News document object.
   *
   * @var \ChapterThree\AppleNews\Document
   */
  public $document;

  /**
   * Implements __construct().
   *
   * @param string $module
   *   The defining module.
   * @param string $machine_name
   *   The machine name of this object.
   * @param array $arguments
   *   Defined in hook_apple_news_api(), altered by hook_apple_news_api_alter().
   */
  public function __construct($module, $machine_name, array $arguments) {
    $this->module = $module;
    $this->machineName = $machine_name;
    $this->arguments = $arguments;
    $this->settings = [
      'channels' => [],
    ];
  }

  /**
   * Implements __sleep().
   *
   * Exclude $this->document.
   */
  public function __sleep() {
    return [
      'module',
      'machineName',
      'arguments',
      'enabled',
      'settings',
    ];
  }

  /**
   * Getter for module.
   */
  public function getModule() {
    return $this->module;
  }

  /**
   * Getter for machineName.
   */
  public function getMachineName() {
    return $this->machineName;
  }

  /**
   * Define exportable entities.
   *
   * @return EntityFieldQuery
   *   Object that will return exportable entities.
   */
  public function query() {
    $query = new EntityFieldQuery();
    $query->addMetaData('account', user_load(1));
    return $query;
  }

  /**
   * Entity types query() may match on.
   *
   * @return array|NULL
   *   List of entity types, or NULL for all.
   */
  public function entityTypes() {
    return NULL;
  }

  /**
   * Entity bundles query() may match on.
   *
   * @return array|NULL
   *   List of entity types, or NULL for all.
   */
  public function bundles() {
    return NULL;
  }

  /**
   * Provides a configuration form for this export.
   */
  public function getConfigForm($form, &$form_state) {

    $form['enabled'] = [
      '#title' => t('Enabled'),
      '#type' => 'checkbox',
      '#default_value' => $this->enabled,
    ];

    $form['channels'] = [
      '#type' => 'container',
      '#prefix' => '<strong>' . t('Feeds') . '</strong>',
    ];

    $saved_channels = !empty($this->settings['channels']) ? $this->settings['channels'] : [];
    $saved_sections = !empty($this->settings['sections']) ? $this->settings['sections'] : [];

    // Entity form JS helper (hide/show Apple News entity form options).
    drupal_add_js(drupal_get_path('module', 'apple_news') . '/js/form.js');
    $api_channels = apple_news_channels(TRUE);
    foreach ($api_channels as $channel_id => $channel) {
      $form['channels']['channel-' . $channel_id] = [
        '#type' => 'checkbox',
        '#title' => $channel['name'],
        '#attributes' => [
          'data-channel-id' => $channel_id
        ],
        '#required' => TRUE,
        '#default_value' => !empty($saved_channels[$channel_id]) ? TRUE : FALSE,
      ];
      //if (count($channel['sections']) > 1) {
        foreach ($channel['sections'] as $section_id => $section) {
          $form['channels']['section-' . $channel_id . '-' . $section_id] = [
            '#type' => 'checkbox',
            '#title' => $section['name'] . ' ' . (!empty($section['default']) ? t('<em>(default)</em>') : ''),
            '#attributes' => [
              'data-section-of' => $channel_id,
              'class' => ['apple-news-sections']
            ],
            '#default_value' => !empty($saved_sections[$section_id]) ? TRUE : FALSE,
          ];
        }
      //}
    }

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
    $this->enabled = $form_state['values']['enabled'];
    $channels = [];
    $sections = [];
    // Save default channel and section options.
    $api_channels = apple_news_channels(TRUE);
    foreach ($api_channels as $channel_id => $channel) {
      if (!empty($form_state['values']['channel-' . $channel_id])) {
        $channels[$channel_id] = TRUE;
      }
      foreach ($channel['sections'] as $section_id => $section) {
        if (!empty($form_state['values']['section-' . $channel_id . '-' . $section_id])) {
          $sections[$section_id] = TRUE;
        }
      }
    }
    // Save in two different arrays for now.
    $this->settings['channels'] = $channels;
    $this->settings['sections'] = $sections;
  }

  /**
   * Exports the entity to an Apple Native Document.
   *
   * @param object $entity
   *   An entity to export.
   *
   * @return NULL|\ChapterThree\AppleNews\Document
   *   A document object, or NULL to skip the export.
   *
   * @todo verify NULL return aborts push.
   */
  public function export($entity) {
    return $this->document;
  }

  /**
   * Exports entity assets.
   *
   * @param object $entity
   *   An entity to export.
   *
   * @return array
   *   Array of assets (images)
   */
  public function assets($entity) {
    return [];
  }

}

/**
 * An export with user-configurable data mapping.
 */
abstract class ApplenewsExportConfigurable extends ApplenewsExport {

  /**
   * ApplenewsDestination objects.
   *
   * @var array
   */
  public $destinations = [];

  /**
   * {@inheritdoc}
   */
  public function __construct($module, $machine_name, array $arguments) {
    parent::__construct($module, $machine_name, $arguments);
    $this->destinations += [
      'components' => [],
    ];
    $this->init();
  }

  /**
   * Implements __sleep().
   */
  public function __sleep() {
    return array_merge(parent::__sleep(), ['destinations']);
  }

  /**
   * Implements __wakeup().
   */
  public function __wakeup() {
    $this->init();
  }

  /**
   * Copy some attribute data to child objects.
   */
  protected function init() {
    foreach ($this->destinations as $key => $destinations) {
      /** @var ApplenewsDestination $obj */
      foreach ($destinations as $index => $obj) {
        $obj->export = $this;
        $obj->depth = $this->destinationDepth($key, $index);
      }
    }
  }

  /**
   * Calculate depth of a destination object.
   */
  protected function destinationDepth($key, $id) {
    $pid = @$this->destinations[$key][$id]->pid;
    if ($pid !== NULL) {
      return $this->destinationDepth($key, $pid) + 1;
    }
    return 0;
  }

  /**
   * Gets the component with id $index.
   *
   * @param int $index
   *   ID of component.
   *
   * @return ApplenewsDestination|NULL
   *   The component destination object.
   */
  public function getComponent($index) {
    return @$this->destinations['components'][$index];
  }

  /**
   * Deletes the component with id $index.
   *
   * @param int $index
   *   ID of component.
   */
  public function deleteComponent($index) {
    unset($this->destinations['components'][$index]);
  }

  /**
   * The set of components, sorted.
   */
  public function getComponents() {
    return $this->getDestinations('components');
  }

  /**
   * A set of destinations, sorted.
   *
   * @return array
   *   A sorted list of destinations.
   */
  protected function getDestinations($key) {
    // Shallow copy of destinations set.
    $destinations = $this->destinations[$key];

    // Sort by hierarchy and weight.
    usort($destinations, function($a, $b) use($key) {
      // Account for hierarchy.
      if ($a->pid !== $b->pid) {
        // Reassign $a or $b until both at same depth.
        if ($a->depth != $b->depth) {
          if ($a->depth > $b->depth) {
            $deeper =& $a;
            $target_depth = $b->depth;
          }
          else {
            $deeper =& $b;
            $target_depth = $a->depth;
          }
          while ($deeper->depth != $target_depth) {
            $deeper = $this->getDestination($key, $deeper->pid);
          }
        }
        // One is parent of the other -- parent before child.
        if ($a == $b) {
          return $deeper == $a ? -1 : 1;
        }
        // Sort by parents.
        $a = $this->getDestination($key, $a->pid);
        $b = $this->getDestination($key, $b->pid);
      }
      // Sort by weight.
      if ($a->weight == $b->weight) {
        return 0;
      }
      return $a->weight < $b->weight ? -1 : 1;
    });

    return $destinations;
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {
    $form = parent::getConfigForm($form, $form_state);

    $form['components'] = [
      '#title' => t('Components'),
      '#tree' => TRUE,
    ];
    /** @var ApplenewsDestination $destination */
    foreach ($this->getdestinations('components') as $destination) {
      $form['components'][$destination->id] = [
        '#apple_news_destination' => $destination,
        'destination' => [
          '#markup' => check_plain($destination->label()),
        ],
        'edit' => [
          '#markup' => l(t('edit'), 'admin/config/content/apple-news/exports/' . apple_news_export_id($this->module, $this->machineName) . '/components/' . $destination->id),
        ],
        'delete' => [
          '#markup' => l(t('delete'), 'admin/config/content/apple-news/exports/' . apple_news_export_id($this->module, $this->machineName) . '/components/' . $destination->id . '/delete'),
        ],
        'id' => [
          '#type' => 'hidden',
          '#default_value' => $destination->id,
          // '#type' => 'textfield',
          // '#size' => 3,
          // '#default_value' => $destination->id,
          // '#disabled' => TRUE,
        ],
        'pid' => [
          '#type' => 'hidden',
          '#default_value' => $destination->pid,
          // '#type' => 'textfield',
          // '#size' => 3,
          // '#default_value' => $destination->pid,
        ],
        'weight' => [
          '#type' => 'weight',
          '#title' => t('Weight'),
          '#default_value' => $destination->weight,
          '#delta' => 50,
          '#title_display' => 'invisible',
        ],
        'depth' => [
          '#type' => 'hidden',
          '#value' => $destination->depth,
        ],
      ];
    }
    $options = apple_news_get_module_apis()['destinations'];
    $options = ['' => t('Select...')] +
      array_map(function($destination) {
        return "${destination['name']} (${destination['module']})";
      }, $options);
    $form['components']['_new'] = [
      '#apple_news_destination' => FALSE,
      'destination' => [
        '#title' => t('Add new component'),
        '#type' => 'select',
        '#options' => $options,
      ],
      'edit' => [
        '#markup' => '',
      ],
      'delete' => [
        '#markup' => '',
      ],
      'id' => [
        '#type' => 'hidden',
        '#default_value' => NULL,
        // '#type' => 'textfield',
        // '#size' => 3,
        // '#default_value' => NULL,
        // '#disabled' => TRUE,
      ],
      'pid' => [
        '#type' => 'hidden',
        '#default_value' => NULL,
        // '#type' => 'textfield',
        // '#size' => 3,
        // '#default_value' => NULL,
      ],
      'weight' => [
        '#type' => 'weight',
        '#title' => t('Weight'),
        '#default_value' => 50,
        '#delta' => 50,
        '#title_display' => 'invisible',
      ],
      'depth' => [
        '#type' => 'hidden',
        '#value' => 0,
      ],
    ];

    return $form;
  }

  /**
   * Submit handler for the configuration form for this export.
   */
  public function submitConfigForm($form, &$form_state) {
    $id = 0;
    $components = &$form_state['values']['components'];
    /** @var ApplenewsDestination $component */
    foreach ($this->destinations['components'] as $id => $component) {
      $component->weight = $components[$component->id]['weight'];
      $component->pid = $components[$component->id]['pid'] == '' ? NULL : $components[$component->id]['pid'];
    }
    $id++;
    if ($components['_new']['destination']) {
      $destinations = apple_news_get_module_apis()['destinations'];
      $info = $destinations[$components['_new']['destination']];
      $destination = apple_news_get_destination($info['module'], $info['machine_name']);
      $destination->weight = $components['_new']['weight'];
      $destination->pid = $components['_new']['pid'] == '' ? NULL : $components['_new']['pid'];
      $destination->id = $id;
      $this->destinations['components'][$id] = $destination;
    }
    parent::submitConfigForm($form, $form_state);
  }

}

/**
 * A configurable node export.
 */
class ApplenewsExportNode extends ApplenewsExportConfigurable {

  /**
   * Bundles to export.
   *
   * @var array
   */
  public $bundles;

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
   * Implements __sleep().
   */
  public function __sleep() {
    return array_merge(parent::__sleep(), ['bundles']);
  }

  /**
   * {@inheritdoc}
   */
  public function query() {
    return parent::query()
      ->entityCondition('entity_type', 'node')
      ->entityCondition('bundle', $this->bundles)
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
    return $this->bundles;
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm($form, &$form_state) {
    $form = parent::getConfigForm($form, $form_state);

    $options = entity_get_info('node')['bundles'];
    $options = array_map(function($info) {
      return $info['label'];
    }, $options);
    $form['bundles'] = [
      '#required'      => TRUE,
      '#title'         => t('Content Types'),
      '#type'          => 'select',
      '#multiple'      => TRUE,
      '#options'       => $options,
      '#default_value' => $this->bundles,
    ];

    $form['layout'] = [
      '#type'          => 'select',
      '#title'         => t('Layout'),
      '#default_value' => $this->settings['layout'],
      '#options'       => [
        'simple' => t('Simple'),
      ],
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigForm($form, &$form_state) {
    $this->bundles = array_filter($form_state['values']['bundles']);
    parent::submitConfigForm($form, $form_state);
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

    switch ($this->settings['layout']) {

      case 'simple':
      default:
        $layout = new Document\Layouts\Layout(7, 1024);
        $this->document = new Document($id, $title,
          apple_news_language_code($language), $layout);
        // Components.
        /** @var ApplenewsDestination $destination */
        foreach ($this->getComponents() as $destination) {
          foreach ($destination->export($entity) as $component) {
            $this->document->addComponent($component);
          }
        }
        break;

    }

    return parent::export($entity);
  }

}
