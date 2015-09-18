<?php

/**
 * @file
 * Export classes.
 */

use ChapterThree\AppleNewsAPI\Document;

/**
 * Base class for exporting Drupal entities.
 */
abstract class ApplenewsExport {

  // @see __construct().
  protected $module;
  protected $machineName;
  protected $arguments;

  /**
   * Export id.
   *
   * @var int
   */
  public $id;

  /**
   * Administrative title.
   *
   * @var string
   */
  public $title;

  /**
   * Administrative description.
   *
   * @var string
   */
  public $description;

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
   * @var \ChapterThree\AppleNewsAPI\Document
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
   *   Defined in hook_applenews_api(), altered by hook_applenews_api_alter().
   */
  public function __construct($module, $machine_name, array $arguments) {
    $this->module = $module;
    $this->machineName = $machine_name;
    $this->arguments = $arguments;
  }

  /**
   * Implements __sleep().
   *
   * Note excluded attributes, $this->id, $this->document, etc.
   */
  public function __sleep() {
    return [
      'module',
      'machineName',
      'arguments',
      'title',
      'description',
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
    $bundles = $this->bundles();
    return $entity_type == $this->entityType() && (
      $bundles === NULL ||
      in_array($bundle, $bundles)
    );
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
      'channels'   => [],
      'sections'   => [],
      'is_preview' => TRUE,
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
   * @param array $values
   *   The Drupal FAPI $form_state['values'] entry for the elements returned by
   *   this function.
   */
  public function getConfigForm(array $values = []) {

    $form['additional_settings'] = [
      '#type' => 'vertical_tabs',
      '#weight' => 99,
      '#attached' => [
        'js' => [drupal_get_path('module', 'applenews') . '/js/export-settings.js']
      ]
    ];

    $form['additional_settings']['general_tab'] = [
      '#type' => 'fieldset',
      '#title' => t('General Settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#group' => 'main',
      '#weight' => -10,
    ];
    $tab = &$form['additional_settings']['general_tab'];

    $tab['title'] = [
      '#type' => 'textfield',
      '#title' => t('Administrative Title'),
      '#default_value' => $this->title,
      '#description' => t('Export name.'),
      '#required' => TRUE,
    ];

    $tab['description'] = [
      '#type' => 'textfield',
      '#title' => t('Administrative Description'),
      '#default_value' => $this->description,
      '#description' => t('Export description.'),
    ];

    $tab['enabled'] = [
      '#title' => t('Enable this export'),
      '#type' => 'checkbox',
      '#default_value' => $this->enabled,
      '#description' => t('This export will not publish to Apple News until enabled.'),
      '#prefix' => '<hr/><br/><strong>' . t('Status') . '</strong>',
      '#weight' => 10,
    ];

    $form['additional_settings']['metadata_tab'] = [
      '#type' => 'fieldset',
      '#title' => t('Metadata'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#group' => 'main',
      '#weight' => -9,
    ];
    $tab = &$form['additional_settings']['metadata_tab'];

    $tab['channels'] = [
      '#type' => 'container',
      '#prefix' => '<strong>' . t('Default channels and sections') . '</strong>',
      '#weight' => 0,
    ];

    $saved_channels = $this->getSetting('channels');
    $saved_sections = $this->getSetting('sections');

    // Entity form JS helper (hide/show Apple News entity form options).
    drupal_add_js(drupal_get_path('module', 'applenews') . '/js/form.js');
    $api_channels = applenews_channels(TRUE);
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
              'class'           => ['applenews-sections'],
            ],
            '#default_value' => !empty($saved_sections[$section_id]) ? TRUE : FALSE,
          ];
        }
      }
      $tab['channels'][] = [
        '#markup' => '<div class="description">' . t('Default Apple News channels and sections where selected content types will be published. This can be overridden on individual entity edit pages.') . '</div>',
      ];
    }
    else {
      $tab['channels']['info'] = [
        '#markup' => t('Please provide Apple News !credentials and add at least one !channel.',
          [
            '!credentials' => l(t('credentials'), 'admin/config/content/applenews/settings'),
            '!channel'     => l(t('channel'), 'admin/config/content/applenews/settings/channels'),
          ]
        )
      ];
    }

    $tab['is_preview'] = [
      '#title' => t('Exported articles will be visible to members of my channel only.'),
      '#type' => 'checkbox',
      '#default_value' => $this->getSetting('is_preview'),
      '#description' => t('Indicates whether this article should be public (live) or should be a preview that is only visible to members of your channel. Uncheck this to publish the article right away and make it visible to all News users. <br/><strong>Note:</strong>  If your channel has not yet been approved to publish articles in Apple News Format, unchecking this option will result in an error.'),
      '#prefix' => '<strong>' . t('Content visibility') . '</strong>',
      '#weight' => 1,
    ];

    return $form;
  }

  /**
   * Validation handler for the configuration form for this export.
   *
   * @param array $values
   *   The Drupal FAPI $form_state['values'] entry for the elements returned by
   *   this function.
   */
  public function validateConfigForm(array $values) {
  }

  /**
   * Submit handler for the configuration form for this export.
   *
   * @param array $values
   *   The Drupal FAPI $form_state['values'] entry for the elements returned by
   *   this function.
   */
  public function submitConfigForm(array $values) {
    $this->title = $values['additional_settings']['general_tab']['title'];
    $this->description = $values['additional_settings']['general_tab']['description'];
    $this->enabled = $values['additional_settings']['general_tab']['enabled'];

    $channels = [];
    $sections = [];
    // Save default channel and section options.
    $api_channels = applenews_channels(TRUE);
    $channels_element = &$values['additional_settings']['metadata_tab']['channels'];
    foreach ($api_channels as $channel_id => $channel) {
      if (!empty($channels_element['channel-' . $channel_id])) {
        $channels[$channel_id] = TRUE;
      }
      foreach ($channel['sections'] as $section_id => $section) {
        if (!empty($channels_element['section-' . $channel_id . '-' . $section_id])) {
          $sections[$section_id] = TRUE;
        }
      }
    }
    // Save in two different arrays for now.
    $this->setSetting('channels', $channels)
      ->setSetting('sections', $sections);

    // Save isPreview metadata.
    $this->setSetting('is_preview', (bool) $values['additional_settings']['metadata_tab']['is_preview']);

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
   * Exports the entity to an Apple News Document object.
   *
   * @return NULL|\ChapterThree\AppleNewsAPI\Document
   *   A document object, or NULL to skip the export.
   *
   * @todo verify NULL return aborts push.
   */
  abstract public function exportDocument();

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
          // Only need to prevent any spaces in filename -- but let's be a
          // little aggressive here with allowed extension characters.
          if (preg_match("/^[a-zA-Z0-9]+$/", $ext)) {
            $url .= ".${ext}";
          }
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
  public $destinations = [];

  /**
   * {@inheritdoc}
   */
  public function __construct($module, $machine_name, array $arguments) {
    parent::__construct($module, $machine_name, $arguments);
    $this->destinations = [
      'metadata' => NULL,
      'components' => [],
      'layout' => NULL,
      'componentLayouts' => [],
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
    // Prevent un-serialization of no-longer defined classes.
    foreach ($this->destinations['components'] as $index => $obj) {
      if ($obj instanceof __PHP_Incomplete_Class ||
        !$obj instanceof ApplenewsDestinationComponent
      ) {
        unset($this->destinations['components'][$index]);
      }
    }
    foreach ($this->destinations['componentLayouts'] as $index => $obj) {
      if ($obj instanceof __PHP_Incomplete_Class ||
        !$obj instanceof ApplenewsDestinationComponentLayout
      ) {
        unset($this->destinations['componentLayouts'][$index]);
      }
    }
    $obj = $this->destinations['metadata'];
    if ($obj instanceof __PHP_Incomplete_Class ||
      !$obj instanceof ApplenewsDestinationMetadata
    ) {
      unset($this->destinations['metadata']);
    }
    $obj = $this->destinations['layout'];
    if ($obj instanceof __PHP_Incomplete_Class ||
      !$obj instanceof ApplenewsDestinationLayout
    ) {
      unset($this->destinations['layout']);
    }
    $this->init();
  }

  /**
   * Initialize some attributes.
   */
  protected function init() {
    // Set some defaults.
    if (empty($this->destinations['layout'])) {
      $this->setLayout($this->newLayout());
    }
    if (!$this->getMetadata()) {
      $this->setMetadata($this->newMetadata());
    }
    // Copy some attribute data to child objects.
    foreach (['components', 'componentLayouts'] as $key) {
      /** @var ApplenewsDestination $obj */
      foreach ($this->destinations[$key] as $index => $obj) {
        $obj->export = $this;
        $obj->depth = $this->destinationDepth($key, $index);
      }
    }
    $this->getLayout()->export = $this;
    $this->getMetadata()->export = $this;
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
   * Generates a new metadata attribute.
   *
   * @return ApplenewsDestinationMetadata
   *   A new componentLayout object.
   */
  public function newMetadata() {
    module_load_include('inc', 'applenews');
    return applenews_new_destination('applenews', 'metadata');
  }

  /**
   * Sets the metadata destination.
   */
  public function setMetadata(ApplenewsDestinationMetadata $metadata) {
    $this->destinations['metadata'] = $metadata;
  }

  /**
   * Gets the metadata destination.
   *
   * @return ApplenewsDestinationMetadata
   *   Metadata destination object.
   */
  public function getMetadata() {
    return $this->destinations['metadata'];
  }

  /**
   * Generates a new layout attribute.
   *
   * @return ApplenewsDestinationLayout
   *   A new componentLayout object.
   */
  public function newLayout() {
    module_load_include('inc', 'applenews');
    return applenews_new_destination('applenews', 'layout');
  }

  /**
   * Sets the layout destination.
   */
  public function setLayout(ApplenewsDestinationLayout $layout) {
    $this->destinations['layout'] = $layout;
  }

  /**
   * Get layout.
   *
   * @return ApplenewsDestinationLayout
   *   Layout object.
   */
  public function getLayout() {
    return $this->destinations['layout'];
  }

  /**
   * Generates a new componentLayout object.
   *
   * @return ApplenewsDestinationComponentLayout
   *   A new componentLayout object.
   */
  public function newComponentLayout() {
    module_load_include('inc', 'applenews');
    return applenews_new_destination('applenews', 'componentLayout');
  }

  /**
   * Gets the componentLayout with id $index.
   *
   * @param int|string $index
   *   ID of componentLayout.
   *
   * @return ApplenewsDestinationComponentLayout|NULL
   *   The componentLayout destination object.
   */
  public function getComponentLayout($index) {
    return @$this->destinations['componentLayouts'][$index];
  }

  /**
   * Sets the componentLayout to id $index.
   *
   * @param ApplenewsDestinationComponentLayout $component_layout
   *   ComponentLayout object.
   * @param int $index
   *   ID of componentLayout.
   *
   * @return $this
   */
  public function addComponentLayout(ApplenewsDestinationComponentLayout $component_layout, $index = NULL) {
    return $this->addDestination('componentLayouts', $component_layout, $index);
  }

  /**
   * Deletes the componentLayout with id $index.
   *
   * @param int $index
   *   ID of componentLayout.
   */
  public function deleteComponentLayout($index) {
    unset($this->destinations['componentLayouts'][$index]);
  }

  /**
   * The set of componentLayouts, sorted.
   */
  public function getComponentLayouts() {
    return $this->getDestinations('componentLayouts');
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
   * Adds a component.
   *
   * @param \ApplenewsDestinationComponent $component
   *   The component.
   * @param int $index
   *   ID of the component.
   *
   * @return $this
   */
  public function addComponent(ApplenewsDestinationComponent $component, $index = NULL) {
    return $this->addDestination('components', $component, $index);
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
   * Gets ordered children of a component.
   *
   * @param int $index
   *   ID of component.
   */
  public function getChildComponents($index) {
    return $this->getDestinationChildren('components', $index);
  }

  /**
   * Calls drupal_set_message() for any components missing required config.
   */
  public function validateComponents() {
    /** @var ApplenewsDestinationComponent $destination */
    foreach ($this->getdestinations('components') as $destination) {
      if (!$destination->validateSettings()) {
        $label = $this->title ? $this->title :
          t('Export (!id)', ['!id' => $this->id]);
        $edit = 'admin/config/content/applenews/export/' . $this->id .
          '/component/' . $destination->id;
        $msg = t('%export component %label needs to be !link.', [
          '%export' => $label,
          '%label' => $destination->label(),
          '!link' => l(t('configured'), $edit)
        ]);
        drupal_set_message($msg, 'error', FALSE);
      }
    }
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
      $index = count($this->destinations[$key]) + 1;
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
   * Get children of a destination.
   *
   * @param string $key
   *   A key in $this->destinations.
   * @param int $index
   *   ID of component.
   *
   * @return array
   *   A sorted list of destinations.
   */
  protected function getDestinationChildren($key, $index) {
    $children = [];

    if (isset($this->destinations[$key][$index])) {
      /** @var ApplenewsDestination $parent */
      $parent = $this->destinations[$key][$index];
      if ($parent->isLeaf()) {
        return [];
      }
      $can_break = FALSE;
      /** @var ApplenewsDestination $destination */
      foreach ($this->getDestinations($key) as $destination) {
        if ($destination->pid == $index) {
          $children[] = $destination;
          $can_break = TRUE;
        }
        elseif ($can_break) {
          break;
        }
      }
    }

    return $children;
  }

  /**
   * {@inheritdoc}
   */
  public function getConfigForm(array $values = []) {
    $form = parent::getConfigForm($values);

    $tab = &$form['additional_settings']['metadata_tab'];

    // Apple News document metadata (not publisher api metadata).
    $child_values = isset($values['metadata_destination']) ? $values['metadata_destination'] : [];
    $tab['metadata_destination'] = [
        '#type' => 'fieldset',
        '#title' => t('Document Metadata'),
        '#tree' => TRUE,
        '#weight' => 10,
      ] +
      $this->getMetadata()->getConfigForm($child_values);

    $form['additional_settings']['layouts_tab'] = [
      '#type' => 'fieldset',
      '#title' => t('Layout'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#group' => 'main',
    ];
    $tab = &$form['additional_settings']['layouts_tab'];

    // Layouts.
    $child_values = isset($values['layout_destination']) ? $values['layout_destination'] : [];
    $tab['layout_destination'] = [
        '#type' => 'fieldset',
        '#title' => t('Document Layout'),
        '#tree' => TRUE,
        '#weight' => 1,
      ] +
      $this->getLayout()->getConfigForm($child_values);

    // Component Layouts.
    $tab['component_layouts'] = [
      '#title'  => t('Component Layouts'),
      '#tree'   => TRUE,
      '#weight' => 2,
    ];
    /** @var ApplenewsDestinationComponentLayout $destination */
    foreach ($this->getdestinations('componentLayouts') as $destination) {
      $label = check_plain($destination->label());
      if (!$destination->isLeaf()) {
        $label = "<strong>${label}</strong>";
      }
      $edit = 'admin/config/content/applenews/export/' . $this->id . '/componentLayout/' . $destination->id;
      if (!$destination->validateSettings()) {
        drupal_set_message(t('The %label component layout needs to be !link.',
          [
            '%label' => check_plain($destination->label()),
            '!link' => l(t('configured'), $edit)
          ]),
          'error');
      }
      $tab['component_layouts'][$destination->id] = [
        '#applenews_destination' => $destination,
        'destination' => [
          '#markup' => $label,
        ],
        'edit' => [
          '#markup' => l(t('edit'), $edit),
        ],
        'delete' => [
          '#markup' => l(t('delete'), 'admin/config/content/applenews/export/' . $this->id . '/componentLayout/' . $destination->id . '/delete'),
        ],
        'id' => [
          '#type' => 'hidden',
          '#default_value' => $destination->id,
          // '#type' => 'textfield',
          // '#size' => 3,
          // '#default_value' => $destination->id,
          // '#disabled' => TRUE,
        ],
      ];
    }
    $tab['component_layouts']['_new'] = [
      '#applenews_destination' => FALSE,
      'destination' => [
        '#title' => t('Add new component layout'),
        '#type' => 'checkbox',
        '#description' => t('To add a new component layout check the box and hit <em>Save changes</em> button.'),
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
    ];

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
      $label = check_plain($destination->label());
      if (!$destination->isLeaf()) {
        $label = "<strong>${label}</strong>";
      }
      $edit = 'admin/config/content/applenews/export/' . $this->id . '/component/' . $destination->id;
      $tab['components'][$destination->id] = [
        '#applenews_destination' => $destination,
        'destination' => [
          '#markup' => $label,
        ],
        'edit' => [
          '#markup' => l(t('edit'), $edit),
        ],
        'delete' => [
          '#markup' => l(t('delete'), 'admin/config/content/applenews/export/' . $this->id . '/component/' . $destination->id . '/delete'),
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
    $options = applenews_get_module_apis('destination', 'component');
    $options = ['' => t('Select...')] +
      array_map(function($destination) {
        return "${destination['name']} (${destination['module']})";
      }, $options);
    $tab['components']['_new'] = [
      '#applenews_destination' => FALSE,
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
  public function submitConfigForm(array $values) {
    $this->getMetadata()->submitConfigForm(
      $values['additional_settings']['metadata_tab']['metadata_destination']);

    $this->getLayout()->submitConfigForm(
      $values['additional_settings']['layouts_tab']['layout_destination']);

    $components = &$values['additional_settings']['components_tab']['components'];
    /** @var ApplenewsDestinationComponent $component */
    foreach ($this->destinations['components'] as $id => $component) {
      $component->weight = (int) $components[$component->id]['weight'];
      $component->pid = $components[$component->id]['pid'] == '' ? NULL
        : (int) $components[$component->id]['pid'];
    }
    if ($components['_new']['destination']) {
      $destinations = applenews_get_module_apis('destination');
      $info = $destinations[$components['_new']['destination']];
      $destination = applenews_new_destination($info['module'], $info['machine_name']);
      $destination->weight = (int) $components['_new']['weight'];
      $destination->pid = $components['_new']['pid'] == '' ? NULL
        : (int) $components['_new']['pid'];
      $destination->id = empty($this->destinations['components']) ? 1
        : max(array_keys($this->destinations['components'])) + 1;
      $this->destinations['components'][$destination->id] = $destination;
      $form_state['redirect'][] = 'admin/config/content/applenews/export/' .
        $this->id . '/component/' . $destination->id;
    }

    $component_layouts = &$values['additional_settings']['layouts_tab']['component_layouts'];
    if ($component_layouts['_new']['destination']) {
      $layout = $this->newComponentLayout();
      $this->addComponentLayout($layout);
      $form_state['redirect'][] = 'admin/config/content/applenews/export/' .
        $this->id . '/componentLayout/' . $layout->id;
    }

    parent::submitConfigForm($values);
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
  public $bundles = [];

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
  public function getConfigForm(array $values = []) {
    $form = parent::getConfigForm($values);

    $tab = &$form['additional_settings']['general_tab'];

    $options = entity_get_info('node')['bundles'];
    $options = array_map(function($info) {
      return $info['label'];
    }, $options);
    $tab['bundles'] = [
      '#required'      => TRUE,
      '#title'         => t('Content types'),
      '#type'          => 'checkboxes',
      '#options'       => $options,
      '#default_value' => $this->bundles,
      '#description'   => t('Please select content types that will be available for Apple News export.'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfigForm(array $values) {
    $this->bundles = array_filter(
      $values['additional_settings']['general_tab']['bundles']);
    parent::submitConfigForm($values);
  }

  /**
   * {@inheritdoc}
   */
  public function exportDocument() {

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
      applenews_language_code($language),
      $this->getLayout()->export());

    /** @var \ChapterThree\AppleNewsAPI\Document\Metadata $metadata */
    $this->document->setMetadata($this->getMetadata()->export());

    // Components.
    /** @var ApplenewsDestinationComponentLayout $destination */
    foreach ($this->getComponentLayouts() as $key => $destination) {
      $this->document
        ->addComponentLayout($destination->id, $destination->export());
    }
    /** @var ApplenewsDestinationComponent $destination */
    foreach ($this->getComponents() as $destination) {
      foreach ($destination->export() as $component) {
        $this->document->addComponent($component);
      }
    }

    return $this->document;
  }

}

/**
 * A pre-configured, configurable node export.
 */
class ApplenewsExportNodePreConfigured extends ApplenewsExportNode {

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

    // Layout.
    /** @var ApplenewsDestinationLayout $layout */
    $layout = applenews_new_destination('applenews', 'layout');
    $layout->setSetting('margin', '75')
      ->setSetting('gutter', '20');
    switch ($arguments['alignment']) {
      case 'center':
        $layout->setSetting('width', '768')
          ->setSetting('columns', '9');
        break;

      case 'left':
      case 'right':
        $layout->setSetting('width', '1024')
          ->setSetting('columns', '7');
        break;
    }
    $this->setLayout($layout);

    // Component Layouts.
    /** @var ApplenewsDestinationComponentLayout $layout */
    $layout = applenews_new_destination('applenews', 'componentLayout');
    $layout->title = "Body";
    $layout->setSetting('margin', [
      'margin' => 'value',
      'top' => '25',
      'bottom' => '25',
    ]);
    switch ($arguments['alignment']) {
      case 'left':
        $layout->setSetting('columnStart', '0')
          ->setSetting('columnSpan', '5');
        break;

      case 'center':
        $layout->setSetting('columnStart', '1')
          ->setSetting('columnSpan', '7');
        break;

      case 'right':
        $layout->setSetting('columnStart', '2')
          ->setSetting('columnSpan', '5');
        break;
    }
    $this->addComponentLayout($layout, 'body');
    /** @var ApplenewsDestinationComponentLayout $layout */
    $layout = applenews_new_destination('applenews', 'componentLayout');
    $layout->title = "Inline Image";
    switch ($arguments['alignment']) {
      case 'left':
        $layout->setSetting('columnStart', '3')
          ->setSetting('columnSpan', '4');
        break;

      case 'center':
        $layout->setSetting('columnStart', '1')
          ->setSetting('columnSpan', '7');
        break;

      case 'right':
        $layout->setSetting('columnStart', '0')
          ->setSetting('columnSpan', '4');
        break;
    }
    $this->addComponentLayout($layout, 'inline_image');
    /** @var ApplenewsDestinationComponentLayout $layout */
    $layout = applenews_new_destination('applenews', 'componentLayout');
    $layout->title = "Image Set";
    $layout->setSetting('ignoreDocumentMargin', 'left')
      ->setSetting('margin', [
        'margin' => 'value',
        'top' => '25',
        'bottom' => '25',
      ]);
    switch ($arguments['alignment']) {
      case 'center':
        $layout->setSetting('columnStart', '0')
          ->setSetting('columnSpan', '9');
        break;

      case 'left':
      case 'right':
        $layout->setSetting('columnStart', '0')
          ->setSetting('columnSpan', '7');
        break;
    }
    $this->addComponentLayout($layout, 'image_set');

    // Components.
    /** @var ApplenewsDestinationText $component */
    $component = applenews_new_destination('applenews', 'text');
    $component->weight = '0';
    $component->title = 'Title';
    $component
      ->setSetting('text', [
        'source' => '::::applenews::node::label',
        'value' => '',
      ])
      ->setSetting('component', 'title')
      ->setSetting('heading', '')
      ->setSetting('format', '');
    $this->addComponent($component);
    /** @var ApplenewsDestinationBodyPhoto $component */
    $component = applenews_new_destination('applenews', 'bodyphoto');
    $component->weight = '1';
    $component->title = 'Body';
    $component->setLayout('body')
      ->setLayout('inline_image', 'image')
      ->setSetting('text', [
        'source' => '::::applenews::field::node::body::value_sanitized',
        'value' => '',
      ])
      ->setSetting('format', 'markdown');
    $this->addComponent($component);
    /** @var ApplenewsDestinationImages $component */
    $component = applenews_new_destination('applenews', 'images');
    $component->weight = '2';
    $component->title = 'Images';
    $component
      ->setLayout('image_set')
      ->setSetting('type', 'gallery')
      ->setSetting('items', [
        'source' => '::::applenews::file::node::field_image::uri',
        'value' => '',
      ])
      ->setSetting('caption', [
        'source' => '::::applenews::file::node::field_image::alt',
        'value' => '',
      ])
      ->setSetting('format', 'markdown');
    $this->addComponent($component);

  }

}
