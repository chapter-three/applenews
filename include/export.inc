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
   * User-configurable settings.
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
   * Source object.
   *
   * @var object
   */
  public $source;

  /**
   * Identifier index.
   *
   * @var int
   */
  private $identifierIndex;

  /**
   * Assets bundled with an export.
   *
   * @var array
   */
  public $assets;

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
    $query->entityCondition('entity_type', $this->entityType())
      ->entityCondition('bundle', $this->bundles());
    return $query;
  }

  /**
   * Entity type query() may match on.
   *
   * @return string
   *   Entity type name.
   */
  abstract public function entityType();

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
   * Show this export as an option on entity edit form?
   *
   * @param string $entity_type
   *   The entity type.
   * @param string $bundle
   *   The entity bundle.
   *
   * @return bool
   *   Show if true.
   */
  public function showOnEdit($entity_type, $bundle) {
    return $entity_type == $this->entityType() &&
      @$this->bundles()[$bundle];
  }

  /**
   * Declare settins and defaults.
   *
   * @return array
   *   Hash of valid keys for getSetting() and setSetting() to their default
   *   value.
   */
  public function settings() {
    return [
      'channels' => [],
      'sections' => [],
    ];
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
   * @return ApplenewsExport|bool
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
   * Provides a configuration form for this export.
   *
   * @param array $form
   *   FAPI $form.
   * @param array $form_state
   *   FAPI $form_state.
   *
   * @return array
   *   FAPI $form.
   */
  public function getConfigForm(array $form, array &$form_state) {

    $form['additional_settings'] = [
      '#type' => 'vertical_tabs',
      '#weight' => 99,
      '#attached' => [
        'js' => [drupal_get_path('module', 'apple_news') . '/js/export-settings.js']
      ]
    ];

    $form['additional_settings']['general'] = [
      '#type' => 'fieldset',
      '#title' => t('General Settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#group' => 'main',
      '#weight' => -10,
    ];

    $form['additional_settings']['general']['enabled'] = [
      '#title' => t('Enable this export'),
      '#type' => 'checkbox',
      '#default_value' => $this->enabled,
      '#description' => t('This export will not publish to Apple News until enabled.'),
      '#prefix' => '<br/><hr/><br/><strong>' . t('Status') . '</strong>',
      '#weight' => 10,
    ];

    $tab = &$form['additional_settings']['general'];

    $tab['channels'] = [
      '#type' => 'container',
      '#prefix' => '<strong>' . t('Channels') . '</strong>',
      '#weight' => 9,
    ];

    $saved_channels = $this->getSetting('channels');
    $saved_sections = $this->getSetting('sections');

    // Entity form JS helper (hide/show Apple News entity form options).
    drupal_add_js(drupal_get_path('module', 'apple_news') . '/js/form.js');
    $api_channels = apple_news_channels(TRUE);
    if (!empty($api_channels)) {
      foreach ($api_channels as $channel_id => $channel) {
        $tab['channels']['channel-' . $channel_id] = [
          '#type' => 'checkbox',
          '#title' => $channel['name'],
          '#attributes' => [
            'data-channel-id' => $channel_id
          ],
          '#default_value' => !empty($saved_channels[$channel_id]) ? TRUE : FALSE,
        ];
        foreach ($channel['sections'] as $section_id => $section) {
          $tab['channels']['section-' . $channel_id . '-' . $section_id] = [
            '#type' => 'checkbox',
            '#title' => $section['name'] . ' ' . (!empty($section['default']) ? t('<em>(default)</em>') : ''),
            '#attributes' => [
              'data-section-of' => $channel_id,
              'class'           => ['apple-news-sections'],
            ],
            '#default_value' => !empty($saved_sections[$section_id]) ? TRUE : FALSE,
          ];
        }
      }
    }
    else {
      $tab['channels']['info'] = [
        '#markup' => t('Please provide Apple News !credentials and add at least one !channel.',
          [
            '!credentials' => l(t('credentials'), 'admin/config/content/apple-news/settings'),
            '!channel'     => l(t('channel'), 'admin/config/content/apple-news/settings/channels'),
          ]
        )
      ];
    }

    return $form;
  }

  /**
   * Validation handler for the configuration form for this export.
   *
   * @param array $form
   *   FAPI $form.
   * @param array $form_state
   *   FAPI $form_state.
   */
  public function validateConfigForm(array $form, array &$form_state) {
  }

  /**
   * Submit handler for the configuration form for this export.
   *
   * @param array $form
   *   FAPI $form.
   * @param array $form_state
   *   FAPI $form_state.
   */
  public function submitConfigForm(array $form, array &$form_state) {
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
    $this->setSetting('channels', $channels)
      ->setSetting('sections', $sections);

    foreach (array_keys($this->settings()) as $key) {
      if ($key != 'channels' && $key != 'sections' &&
          isset($form_state['values'][$key])
      ) {
        $this->setSetting($key, $form_state['values'][$key]);
      }
    }

  }

  /**
   * Set the source prior to export.
   *
   * @param object $entity
   *   An entity to export.
   *
   * @return $this
   */
  public function setSource($entity) {
    $this->assets = [];
    $this->document = NULL;
    $this->source = $entity;
    $this->identifierIndex = 1;
    return $this;
  }

  /**
   * Exports the entity to an Apple Native Document.
   *
   * @return NULL|\ChapterThree\AppleNews\Document
   *   A document object, or NULL to skip the export.
   *
   * @todo verify NULL return aborts push.
   */
  abstract public function export();

  /**
   * Generates a unique identifier each time called..
   *
   * @return string
   *   An identifier string.
   */
  public function generateIdentifier() {
    return '__' . $this->identifierIndex++;
  }

  /**
   * Generates a News Document URL for an asset.
   *
   * @param string $uri
   *   URI of the file -- if no scheme assumed to be local link.
   */
  public function addAsset($uri) {
    if (!isset($this->assets[$uri])) {
      $path = FALSE;
      // A drupal uri.
      if (file_valid_uri($uri)) {
        /** @var DrupalStreamWrapperInterface $wrapper */
        if ($wrapper = file_stream_wrapper_get_instance_by_uri($uri)) {
          $path = $wrapper->realPath();
        }
      }
      else {
        global $base_url;
        $scheme = parse_url($uri, PHP_URL_SCHEME);
        // Handle uri to local file.
        if (parse_url($uri, PHP_URL_HOST) == parse_url($base_url, PHP_URL_HOST)) {
          $scheme = '';
        }
        // A local link.
        if ($scheme == '') {
          $path = drupal_realpath(ltrim(parse_url($uri, PHP_URL_PATH), '/'));
        }
        // External, try to fetch.
        else {
          $path = drupal_realpath(system_retrieve_file($uri));
        }
      }
      if ($path) {
        $url = 'bundle://' . count($this->assets);
        if ($ext = pathinfo($path, PATHINFO_EXTENSION)) {
          $url .= ".${ext}";
        }
        $this->assets[$uri] = [
          'path' => $path,
          'url' => $url,
        ];
      }
    }
    return @$this->assets[$uri]['url'];
  }

  /**
   * Get assets associated with an export.
   *
   * Note that running export() will populate the data for this method. If ran
   * on an instance where export() has not run, will return NULL.
   *
   * @return array|NULL
   *   Array of asset paths keyed on Apple Document URL.
   *
   * @see addAsset()
   */
  public function getAssets() {
    $files = [];
    foreach ($this->assets as $info) {
      $files[$info['url']] = $info['path'];
    }
    return $files;
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
  public $destinations = [
    'components' => [],
  ];

  /**
   * {@inheritdoc}
   */
  public function __construct($module, $machine_name, array $arguments) {
    parent::__construct($module, $machine_name, $arguments);
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
    // Prevent unserialization of no-longer defined classes.
    foreach ($this->destinations as $key => $destinations) {
      foreach ($destinations as $index => $obj) {
        if ($obj instanceof __PHP_Incomplete_Class ||
            !$obj instanceof ApplenewsDestination
        ) {
          unset($this->destinations[$key][$index]);
        }
      }
    }
    $this->init();
  }

  /**
   * Initialize some attributes.
   */
  protected function init() {
    // Copy some attribute data to child objects.
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
   *
   * @param string $key
   *   A key in $this->destinations.
   * @param string|int $id
   *   Id of destination.
   *
   * @return int
   *   The depth.
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
   * @return ApplenewsDestinationComponent|NULL
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
   * Sets the destination to id $index.
   *
   * @param string $key
   *   Destination type.
   * @param ApplenewsDestination $destination
   *   ComponentLayout object.
   * @param int|string $index
   *   ID of componentLayout.
   *
   * @return $this
   */
  protected function addDestination($key, ApplenewsDestination $destination, $index = NULL) {
    if (!isset($index)) {
      $index = count($this->destinations[$key]);
      while (isset($this->destinations[$key][$index])) {
        $index++;
      }
    }
    $destination->id = $index;
    $this->destinations[$key][$index] = $destination;
    return $this;
  }

  /**
   * A set of destinations, sorted.
   *
   * @param string $key
   *   A key in $this->destinations.
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
      if ($a->depth != $b->depth) {
        // Walk deeper item up the tree until at same depth.
        if ($a->depth < $b->depth) {
          $deeper = $b;
          while ($deeper->depth != $a->depth) {
            $deeper = $this->destinations[$key][$deeper->pid];
          }
          // $a is ancestor of $b -- parent before child.
          if ($deeper->id == $a->id) {
            return -1;
          }
          // Sort by parent.
          $b = $deeper;
        }
        else {
          $deeper = $a;
          while ($deeper->depth != $b->depth) {
            $deeper = $this->destinations[$key][$deeper->pid];
          }
          // $b is ancestor of $a -- parent before child.
          if ($deeper->id == $b->id) {
            return 1;
          }
          // Sort by parent.
          $a = $deeper;
        }
        // $a and $b now at same depth.
      }
      if ($a->pid !== $b->pid) {
        // Sort by parents.
        $a = $this->destinations[$key][$a->pid];
        $b = $this->destinations[$key][$b->pid];
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
  public function getConfigForm(array $form, array &$form_state) {
    $form = parent::getConfigForm($form, $form_state);

    $form['additional_settings']['layouts_tab'] = [
      '#type' => 'fieldset',
      '#title' => t('Layout'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#group' => 'main',
    ];
    // $tab = &$form['additional_settings']['layouts_tab'];
    // @todo Layout, ComponentLayout.


    $form['additional_settings']['components_tab'] = [
      '#type' => 'fieldset',
      '#title' => t('Components'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#group' => 'main',
    ];
    $tab = &$form['additional_settings']['components_tab'];

    // Components.
    $tab['components'] = [
      '#title'  => t('Components'),
      '#tree'   => TRUE,
    ];
    /** @var ApplenewsDestinationComponent $destination */
    foreach ($this->getdestinations('components') as $destination) {
      $edit = 'admin/config/content/apple-news/exports/' . apple_news_export_id($this->module, $this->machineName) . '/component/' . $destination->id;
      if (!$destination->validateSettings()) {
        drupal_set_message(t('The !label component needs to be !link.',
          [
            '!label' => $destination->label(),
            '!link' => l(t('configured'), $edit)
          ]),
          'error');
      }
      $tab['components'][$destination->id] = [
        '#apple_news_destination' => $destination,
        'destination' => [
          '#markup' => $destination->label(),
        ],
        'edit' => [
          '#markup' => l(t('edit'), $edit),
        ],
        'delete' => [
          '#markup' => l(t('delete'), 'admin/config/content/apple-news/exports/' . apple_news_export_id($this->module, $this->machineName) . '/component/' . $destination->id . '/delete'),
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
    $options = apple_news_get_module_apis('destination', 'component');
    $options = ['' => t('Select...')] +
      array_map(function($destination) {
        return "${destination['name']} (${destination['module']})";
      }, $options);
    $tab['components']['_new'] = [
      '#apple_news_destination' => FALSE,
      'destination' => [
        '#title' => t('Add new component'),
        '#type' => 'select',
        '#options' => $options,
        '#description' => t('To add a new component please select component from the list and hit <em>Save changes</em> button.'),
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
   * {@inheritdoc}
   */
  public function submitConfigForm(array $form, array &$form_state) {
    $components = &$form_state['values']['components'];
    /** @var ApplenewsDestinationComponent $component */
    foreach ($this->destinations['components'] as $id => $component) {
      $component->weight = (int) $components[$component->id]['weight'];
      $component->pid = $components[$component->id]['pid'] == '' ? NULL : (int) $components[$component->id]['pid'];
    }
    if ($components['_new']['destination']) {
      $destinations = apple_news_get_module_apis('destination');
      $info = $destinations[$components['_new']['destination']];
      $destination = apple_news_get_destination($info['module'], $info['machine_name']);
      $destination->weight = (int) $components['_new']['weight'];
      $destination->pid = $components['_new']['pid'] == '' ? NULL : (int) $components['_new']['pid'];
      $destination->id = empty($this->destinations['components']) ? 1 :
        max(array_keys($this->destinations['components'])) + 1;
      $this->destinations['components'][$destination->id] = $destination;
      $form_state['redirect'][] = 'admin/config/content/apple-news/exports/' .
        apple_news_export_id($this->module, $this->machineName) .
        '/component/' . $destination->id;
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
   * Implements __sleep().
   */
  public function __sleep() {
    return array_merge(parent::__sleep(), ['bundles']);
  }

  /**
   * {@inheritdoc}
   */
  public function entityType() {
    return 'node';
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
  public function getConfigForm(array $form, array &$form_state) {
    $form = parent::getConfigForm($form, $form_state);

    $tab = &$form['additional_settings']['general'];

    $options = entity_get_info('node')['bundles'];
    $options = array_map(function($info) {
      return $info['label'];
    }, $options);
    $tab['bundles'] = [
      '#required'      => TRUE,
      '#title'         => t('Content Types'),
      '#type'          => 'checkboxes',
      '#multiple'      => TRUE,
      '#options'       => $options,
      '#default_value' => $this->bundles,
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigForm(array $form, array &$form_state) {
    $this->bundles = array_filter($form_state['values']['bundles']);
    parent::submitConfigForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function export() {

    /** @var EntityDrupalWrapper $wrapper */
    $wrapper = entity_metadata_wrapper('node', $this->source);

    // Identifier;
    $id = $wrapper->getIdentifier();

    // Title.
    $title = $wrapper->label();

    // Language code.
    /** @var EntityValueWrapper $language */
    $language = $wrapper->get('language');
    $language = $language->value();
    if ($language == LANGUAGE_NONE) {
      /** @var stdClass $language */
      $language = language_default();
      $language = $language->language;
    }

    $this->document = new Document($id, $title,
      apple_news_language_code($language),
      // @todo Make these arbitrary values configurable.
      new Document\Layouts\Layout(7, 1024));

    // Components.
    /** @var ApplenewsDestinationComponent $destination */
    foreach ($this->getComponents() as $destination) {
      foreach ($destination->export() as $component) {
        $this->document->addComponent($component);
      }
    }

    return $this->document;
  }

}
