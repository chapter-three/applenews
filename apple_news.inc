<?php

/**
 * @file
 * Helper functions.
 */

/**
 * Defines an export unique key.
 */
function apple_news_export_id($module, $machine_name) {
  return "${module}::${machine_name}";
}

/**
 * Decodes an export id.
 */
function apple_news_export_id_extract($id) {
  return explode('::', $id);
}

/**
 * Implements OBJECT_save().
 *
 * @param ApplenewsExport $export
 *   Object as returned by apple_news_export_load() or apple_news_get_export().
 *
 * @return bool
 *   Success.
 */
function apple_news_export_save(ApplenewsExport $export) {
  $transaction = db_transaction();
  $msg = t('Could not save export %machine_name defined by %module.');
  $var = [
    '%machine_name' => $export->getMachineName(),
    '%module'       => $export->getModule(),
  ];
  try {
    $update = db_select('apple_news_status', 's')
      ->condition('s.module', $export->getModule())
      ->condition('s.machine_name', $export->getMachineName())
      ->countQuery()->execute()->fetchField();
    if ($update) {
      $success = db_update('apple_news_status')
        ->fields([
          'enabled'  => (int) $export->enabled,
          'instance' => serialize($export),
        ])
        ->condition('module', $export->getModule())
        ->condition('machine_name', $export->getMachineName())
        ->execute();
      if ($success) {
        return TRUE;
      }
      $transaction->rollback();
      watchdog('apple_news', $msg, $var, WATCHDOG_ERROR);
    }
    else {
      db_insert('apple_news_status')
        ->fields([
          'module'       => $export->getModule(),
          'machine_name' => $export->getMachineName(),
          'enabled'      => (int) $export->enabled,
          'instance'     => serialize($export),
        ])
        ->execute();
      return TRUE;
    }
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('apple_news', $e, $msg, $var, WATCHDOG_ERROR);
  }
  return FALSE;
}

/**
 * Gets an instantiated destination object.
 *
 * @param string $module
 *   Module machine name.
 * @param string $machine_name
 *   Destination machine name.
 *
 * @return ApplenewsDestination
 *   The instantiated class specified by the implementation..
 */
function apple_news_get_destination($module, $machine_name) {
  $implementations = apple_news_get_module_apis()['destinations'];
  $key = apple_news_export_id($module, $machine_name);
  if (isset($implementations[$key])) {
    $info = $implementations[$key];
    $reflection = apple_news_validate_class($info['class'],
      'ApplenewsDestination');
    if ($reflection) {
      return $reflection->newInstanceArgs(func_get_args());
    }
  }
  return NULL;
}

/**
 * Gets an instantiated source object.
 *
 * @param string $module
 *   Module machine name.
 * @param string $machine_name
 *   Source machine name.
 * @param mixed $arg,...
 *   Any additional parameters as defined by the implementation constructor.
 *
 * @return ApplenewsSourceInterface
 *   The instantiated class specified by the implementation..
 */
function apple_news_get_source($module, $machine_name, $arg = NULL) {
  $implementations = apple_news_get_module_apis()['sources'];
  $key = apple_news_export_id($module, $machine_name);
  if (isset($implementations[$key])) {
    $info = $implementations[$key];
    $reflection = apple_news_validate_class($info['class'],
      'ApplenewsSourceInterface');
    if ($reflection) {
      $args = func_get_args();
      $args = array_splice($args, 2);
      return $reflection->newInstanceArgs($args);
    }
  }
  return NULL;
}

/**
 * Helper function for validating implementation defined classes.
 *
 * @return ReflectionClass|bool
 *   Valid implementations return ReflectionClass, invalid FALSE.
 */
function apple_news_validate_class($class_name, $is_a = NULL) {
  if (!class_exists($class_name)) {
    $msg = 'Class %class does not exist.';
    $args = ['%class' => $class_name];
    watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
    return FALSE;
  }
  $reflection = new ReflectionClass($class_name);
  if ($reflection->isAbstract()) {
    $msg = 'Class %class is not a concrete class';
    $args = ['%class' => $class_name];
    watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
    return FALSE;
  }
  if ($is_a && !($class_name == $is_a || $reflection->isSubclassOf($is_a))) {
    $msg = 'Class %class is not a subclass of %isa';
    $args = ['%class' => $class_name, '%isa' => $is_a];
    watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
    return FALSE;
  }
  return $reflection;
}

/**
 * Gets an export object.
 *
 * @param string $module
 *   Module machine name.
 * @param string $machine_name
 *   Export machine name.
 *
 * @return ApplenewsExport
 *   The instantiated class configured for the export.
 *
 * @see apple_news_exports()
 */
function apple_news_get_export($module, $machine_name) {
  $exports = apple_news_exports();
  $key = apple_news_export_id($module, $machine_name);
  if (empty($exports[$key])) {
    $msg = t('No export %machine_name defined by %module.');
    $var = [
      '%machine_name' => $machine_name,
      '%module'       => $module,
    ];
    watchdog('apple_news', $msg, $var, WATCHDOG_ERROR);
    return NULL;
  }
  $info = $exports[$key];
  if (!class_exists($info['class'])) {
    $msg = 'No export class %class found.';
    $var = [
      '%class' => $info['class']
    ];
    watchdog('apple_news', $msg, $var, WATCHDOG_ERROR);
    return NULL;
  }

  // Load saved.
  $query = db_select('apple_news_status', 's')
    ->fields('s', ['instance'])
    ->condition('s.module', $module)
    ->condition('s.machine_name', $machine_name);
  if ($row = $query->execute()->fetchObject()) {
    if ($export = unserialize($row->instance)) {
      return $export;
    }
  }

  // Instantiate new export.
  try {
    if (!isset($info['arguments']) || !is_array($info['arguments'])) {
      $info['arguments'] = [];
    }
    return new $info['class']($module, $machine_name, $info['arguments']);
  }
  catch (Exception $e) {
    $msg = t('Export class %class could not be constructed: %message');
    $var = [
      '%class' => $info['class'],
      '%message' => $e->getMessage(),
    ];
    watchdog('apple_news', $msg, $var, WATCHDOG_ERROR);
    return NULL;
  }
}

/**
 * Gets registered exports.
 */
function apple_news_exports($reset = NULL) {
  $exports = &drupal_static(__FUNCTION__);
  if (!empty($exports) && empty($reset)) {
    return $exports;
  }

  $implementations = apple_news_get_module_apis();

  $exports = $implementations['exports'];

  $result = db_select('apple_news_status', 's')
    ->fields('s', ['module', 'machine_name', 'enabled'])
    ->execute();
  while ($row = $result->fetchAssoc()) {
    $key = apple_news_export_id($row['module'], $row['machine_name']);
    // Module still enabled and export still defined.
    if (!isset($exports[$key])) {
      $msg = 'Export %machine_name defined by %module no longer defined.';
      $args = [
        '%machine_name' => $row['machine_name'],
        '%module'       => $row['module'],
      ];
      watchdog('apple_news', $msg, $args, WATCHDOG_INFO);
      continue;
    }
    // Good to go.
    $exports[$key]['#enabled'] = !empty($row['enabled']) ? $row['enabled'] : NULL;
    // Validate class.
    if (!apple_news_validate_class($exports[$key]['class'], 'ApplenewsExport')) {
      continue;
    }
  }
  return $exports;
}

/**
 * Get a list of implementations supporting the current API.
 */
function apple_news_get_module_apis() {

  $validate = function($type, $module, $machine_name, $definition) {
    $args = [
      '!type'         => $type,
      '%machine_name' => $machine_name,
      '%module'       => $module,
    ];
    if (empty($definition['class'])) {
      $msg = '!type %machine_name defined by %module is missing required class';
      watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
      return FALSE;
    }
    if (!preg_match('/^[a-zA-Z0-9-_]+$/', $machine_name)) {
      $msg = '!type machine name "%machine_name" defined by %module must match /^[a-zA-Z0-9-_]$/';
      watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
      return FALSE;
    }
    return TRUE;
  };

  $implementations = [];

  foreach (module_implements('apple_news_api') as $module) {
    $function = $module . '_apple_news_api';
    $info = $function() + [
      'exports'      => [],
      'sources'      => [],
      'destinations' => [],
    ];

    // Check API version.
    if (!isset($info['api']) || $info['api'] != APPLE_NEWS_API_VERSION) {
      $msg = t('%function supports Apple News API version %modversion, Apple News module API version is %version - migration support not loaded.');
      $args = [
        '%function'   => $function,
        '%modversion' => $info['api'],
        '%version'    => APPLE_NEWS_API_VERSION,
      ];
      watchdog('apple_news', $msg, $args, WATCHDOG_NOTICE);
      continue;
    }

    // Exports.
    foreach ($info['exports'] as $machine_name => $export) {
      if (!$validate('Export', $module, $machine_name, $export)) {
        continue;
      }
      $key = apple_news_export_id($module, $machine_name);
      $implementations['exports'][$key] = array_merge([
          'name'        => '',
          'description' => '',
          'arguments'   => [],
        ],
        $export,
        [
          'module'       => $module,
          'machine_name' => $machine_name,
        ]
      );
    }

    // Source classes.
    foreach ($info['sources'] as $machine_name => $source) {
      if (!$validate('Source', $module, $machine_name, $source)) {
        continue;
      }
      $key = apple_news_export_id($module, $machine_name);
      // Set "type" to classify source -- each type will take different
      // different parameters in its constructor.
      // @see ApplenewsDestination::getSource()
      $reflection = apple_news_validate_class($source['class']);
      if ($source['class'] == 'ApplenewsSourceEntity' ||
          $reflection->isSubclassOf('ApplenewsSourceEntity')
      ) {
        $type = 'entity';
      }
      elseif ($source['class'] == 'ApplenewsSourceField' ||
          $reflection->isSubclassOf('ApplenewsSourceField')
      ) {
        $type = 'field';
      }
      else {
        $msg = 'Class %class is not a subclass of ApplenewsSourceEntity or ApplenewsSourceField.';
        $args = ['%class' => $source['class']];
        watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
        return FALSE;
      }
      $implementations['sources'][$key] = array_merge([
          'name'        => '',
          'description' => '',
        ],
        $source,
        [
          'module'       => $module,
          'machine_name' => $machine_name,
          'type'         => $type,
        ]
      );
    }

    // Destination classes.
    foreach ($info['destinations'] as $machine_name => $destination) {
      if (!$validate('Destination', $module, $machine_name, $destination)) {
        continue;
      }
      $key = apple_news_export_id($module, $machine_name);
      $implementations['destinations'][$key] = array_merge([
          'name'        => '',
          'description' => '',
        ],
        $destination, [
          'module'       => $module,
          'machine_name' => $machine_name,
        ]
      );
    }

  }

  // Allow modules to alter the migration information.
  drupal_alter('apple_news_api', $implementations);

  return $implementations;
}

/**
 * Post Article to Apple News.
 *
 * @todo Add Revision support.
 */
function apple_news_post($channel_id, $entity_type, $entity_wrapper, $data) {
  module_load_include('inc', 'apple_news', 'push_api');
  // Make POST request to Apple News via PushAPI.
  $response = apple_news_post_article($channel_id, $data);
  // If successful request.
  if (!isset($response->errors)) {
    // Invoke Apple News insert hooks.
    module_invoke_all('apple_news_insert', $response->data->id, $response->data->revision, $channel_id, $entity_wrapper, $entity_type, $data);
    $record = [
      'entity_type'         => $entity_type,
      'entity_id'           => $entity_wrapper->getIdentifier(),
      // Need to work on revision support.
      'revision_id'         => $entity_wrapper->getIdentifier(),
      'article_id'          => $response->data->id,
      'article_revision_id' => $response->data->revision,
      'channel_id'          => $channel_id,
      'postdate'            => REQUEST_TIME,
      'metadata'            => serialize(
        [
          'shareUrl'     => !empty($response->data->shareUrl) ? $response->data->shareUrl : '',
          'type'         => !empty($response->data->type) ? $response->data->type : '',
          'links'        => !empty($response->data->links) ? $response->data->links : '',
          'isSponsored'  => !empty($response->data->isSponsored) ? $response->data->isSponsored : '',
        ]
      ),
    ];
    drupal_write_record('apple_news_published_entities', $record);
    return TRUE;
  }
  return FALSE;
}

/**
 * Post Article to Apple News.
 *
 * @todo Add Revision support.
 */
function apple_news_update($channel_id, $entity_type, $entity_wrapper, $data) {
  module_load_include('inc', 'apple_news', 'push_api');
  $article = db_select('apple_news_published_entities', 'e')
    ->fields('e', ['article_id', 'article_revision_id'])
    ->condition('entity_type', $entity_type)
    ->condition('channel_id', $channel_id)
    ->condition('entity_id', $entity_wrapper->getIdentifier())
    ->execute()
    ->fetchObject();
  // Make sure article exists.
  if (!empty($article->article_id)) {
    // Add metadata.
    if (!empty($data['metadata'])) {
      $data['metadata'] = json_decode($data['metadata'], true);
    }
    $data['metadata']['data']['revision'] = $article->article_revision_id;
    $data['metadata'] = json_encode($data['metadata']);
    // Make POST request to Apple News via PushAPI.
    $response = apple_news_update_article($article->article_id, $data);
    // If successful request.
    if (!isset($response->errors)) {
      // Invoke Apple News update hooks.
      module_invoke_all('apple_news_update', $article->article_id, $article->article_revision_id, $channel_id, $entity_wrapper, $entity_type, $data);
      db_update('apple_news_published_entities')
        ->fields(
          [
            'article_id'          => $response->data->id,
            'article_revision_id' => $response->data->revision,
            'postdate'            => REQUEST_TIME,
            'metadata'            => serialize(
              [
                'shareUrl'     => !empty($response->data->shareUrl) ? $response->data->shareUrl : '',
                'type'         => !empty($response->data->type) ? $response->data->type : '',
                'links'        => !empty($response->data->links) ? $response->data->links : '',
                'isSponsored'  => !empty($response->data->isSponsored) ? $response->data->isSponsored : '',
              ]
            )
          ]
        )
        ->condition('entity_id', $entity_wrapper->getIdentifier())
        ->condition('article_id', $article->article_id)
        ->condition('article_revision_id', $article->article_revision_id)
        ->condition('channel_id', $channel_id)
        ->execute();
      // @todo implement preview
      //module_load_include('inc', 'apple_news', 'include/preview');
      //$preview = new ChapterThree\AppleNews\Preview\ApplenewsPreview($article->article_id, $data);
      //$preview->archive();
      return TRUE;
    }
  }
  else {
    // Create new Apple News article.
    apple_news_post($channel_id, $entity_type, $entity_wrapper, $data);
  }
  return FALSE;
}

/**
 * Delete Article from Apple News.
 */
function apple_news_delete($channel_id, $entity_type, $entity_wrapper) {
  module_load_include('inc', 'apple_news', 'push_api');
  $article_id = db_select('apple_news_published_entities', 'e')
    ->fields('e', ['article_id'])
    ->condition('entity_type', $entity_type)
    ->condition('channel_id', $channel_id)
    ->condition('entity_id', $entity_wrapper->getIdentifier())
    ->execute()
    ->fetchField();
  // Make DELETE request to Apple News via PushAPI.
  $response = apple_news_delete_article($article_id);
  // If successful request.
  if (empty($response)) {
    // Invoke Apple News delete hooks.
    module_invoke_all('apple_news_delete', $article_id, $channel_id, $entity_wrapper, $entity_type);
    db_delete('apple_news_published_entities')
      ->condition('article_id', $article_id)
      ->execute();
    return TRUE;
  }
  return FALSE;
}

/**
 * Get entity types and bundle names that are available for Apple News export.
 *
 * @deprecated
 *
 * @see apple_news_get_source_options()
 */
function apple_news_get_entity_types() {
  $result = [];
  // Get all available Apple News exportable entity types and bundles.
  foreach (apple_news_exports() as $info) {
    // Load export object.
    $export = apple_news_get_export($info['module'], $info['machine_name']);
    $result[$info['module'] . '::' . $info['machine_name']] = 
    [
      'module'        => $info['module'],
      'machine_name'  => $info['machine_name'],
      'enabled'       => $export->enabled,
      'entity_types'  => $export->entityTypes(),
      'bundles'       => $export->bundles(),
      '#export'       => $export,
    ];
  }
  return $result;
}

/**
 * Perform operations on an entity.
 *
 * @param string $op
 *   Entity operation (insert/update/delete).
 * @param string $channel_id
 *   Apple News channel ID.
 * @param object $entity
 *   Entity object.
 * @param string $type
 *   Entity type.
 */
function apple_news_op($op, $channel_id, $entity, $entity_type, $export = NULL, array $settings = []) {
  $result = FALSE;
  $data = [];
  // Entity object wrapper.
  $wrapper = entity_metadata_wrapper($entity_type, $entity);
  // Generate Apple News Document formatted JSON string.
  if (is_object($export)) {
    $document = $export->export($entity);
    if (count($settings)) {
      // Merge export settings with custom fields from the entity add/edit
      // form.
      // @see apple_news_form_alter().
      $export->settings = array_merge($export->settings, $settings);
    }
    $data = [
      'json' => $document->json(),
      'files' => $export->assets($entity),
      'metadata' => !empty($settings['metadata']) ? json_encode($settings['metadata']) : '',
    ];
  }
  if ($op == 'insert') {
    $result = apple_news_post($channel_id, $entity_type, $wrapper, $data);
  }
  elseif ($op == 'update') {
    $result = apple_news_update($channel_id, $entity_type, $wrapper, $data);
  }
  elseif ($op == 'delete') {
    $result = apple_news_delete($channel_id, $entity_type, $wrapper);
  }
  return $result;
}

/**
 * Converts drupal language code to appropriate Apples News value.
 */
function apple_news_language_code($language_code) {
  // @todo
  return $language_code;
}

/**
 * Form configuration fields.
 */
function apple_news_settings_fields() {
  $config_fields = [
    'apple_news_variable_api_endpoint',
    'apple_news_variable_api_key',
    'apple_news_variable_api_secret',
    'apple_news_variable_api_channels',
  ];
  return $config_fields;
}

/**
 * Get Apple News channels.
 */
function apple_news_channels($grouped = FALSE) {
  // Add Post to Apple News checkbox.
  $channels = [];
  $api_channels = variable_get('apple_news_variable_api_channels', []);
  if ($grouped) {
    foreach ($api_channels as $channel_id) {
      $sections = [];
      $channel = apple_news_channel($channel_id);
      $api_sections = apple_news_sections($channel->id);
      foreach ($api_sections as $section) {
        $sections[$section->id] = [
          'name'    => filter_xss($section->name),
          'url'     => $section->links->self,
          'default' => $section->isDefault,
        ];
      }
      $channels[$channel->id] = [
        'name'     => filter_xss($channel->name),
        'sections' => $sections,
      ];
    }
  }
  else {
    foreach ($api_channels as $channel_id) {
      $channel = apple_news_channel($channel_id);
      $channels[$channel->id] = filter_xss($channel->name);
    }
  }
  return $channels;
}

/**
 * Get Apple News sections.
 *
 * @see apple_news_get_sections().
 */
function apple_news_sections($channel_id) {
  $sections = &drupal_static(__FUNCTION__);
  if (!isset($sections)) {
    if ($cache = cache_get('sections_' . $channel_id, 'cache_apple_news')) {
      $sections = $cache->data;
    }
    else {
      module_load_include('inc', 'apple_news', 'push_api');
      $response = apple_news_get_sections($channel_id);
      if (!empty($response->data)) {
        foreach ($response->data as $section) {
          $sections[] = $section;
        }
      }
      cache_set('sections_' . $channel_id, $sections, 'cache_apple_news');
    }
  }
  return $sections;
}

/**
 * Get Apple News section information.
 */
function apple_news_section($section_id) {
  $sections = apple_news_channels(TRUE);
  foreach ($sections as $channel_id => $channel) {
    if (!empty($channel['sections'][$section_id])) {
      return $channel['sections'][$section_id];
    }
  }
  return FALSE;
}

/**
 * Get Apple News channels.
 *
 * @see apple_news_get_sections().
 */
function apple_news_channel($channel_id) {
  $channel = &drupal_static(__FUNCTION__);
  if (!isset($channel)) {
    if ($cache = cache_get('channel_' . $channel_id, 'cache_apple_news')) {
      $channel = $cache->data;
    }
    else {
      module_load_include('inc', 'apple_news', 'push_api');
      $response = apple_news_get_channel($channel_id);
      if (!isset($response->errors)) {
        if (!empty($response->data)) {
          $channel = $response->data;
          cache_set('channel_' . $channel_id, $channel, 'cache_apple_news');
        }
      }
      else {
        return $response;
      }
    }
  }
  return $channel;
}

/**
 * Load posted entities from {apple_news_published_entities} table.
 *
 * @param object $entity
 *   Entity object.
 * @param string $type
 *   Entity type.
 *
 * @return object
 *   Entity objects.
 */
function apple_news_load_articles($entity, $type) {
  $wrapper = entity_metadata_wrapper($type, $entity);
  $result = db_select('apple_news_published_entities', 'e')
    ->fields('e')
    ->condition('entity_type', $type)
    ->condition('entity_id', $wrapper->getIdentifier())
    ->execute();
  return $result;
}

/**
 * Check if the entity already published to Apple News.
 *
 * @param string $channel_id
 *   Apple News channel ID.
 * @param object $entity_wrapper
 *   Entity wrapper object.
 * @param string $type
 *   Entity type.
 *
 * @return object|NULL
 *   Export log record or FALSE if not results.
 */
function apple_news_is_exported($channel_id, $entity_wrapper, $type) {
  $article = db_select('apple_news_published_entities', 'e')
    ->fields('e')
    ->condition('channel_id', $channel_id)
    ->condition('entity_type', $type)
    ->condition('entity_id', $entity_wrapper->getIdentifier())
    ->execute()
    ->fetchObject();
  if (!empty($article->article_id)) {
    return $article;
  }
  return FALSE;
}

/**
 * Get total number of channels where the given article is published.
 *
 * @param object $entity_wrapper
 *   Entity wrapper object.
 * @param string $type
 *   Entity type.
 *
 * @return int
 *   Total number of channels where the given article is published.
 */
function apple_news_get_channels_count($entity_wrapper, $type) {
  $result = db_select('apple_news_published_entities', 'e')
    ->fields('e')
    ->condition('entity_type', $type)
    ->condition('entity_id', $entity_wrapper->getIdentifier())
    ->execute();
  return $result->rowCount();
}

/**
 * Load Apple News article metadata.
 *
 * @param string $article_id
 *   Apple News article ID.
 * @param string $article_revision_id
 *   Apple News article revision ID.
 *
 * @return array
 *   Metadata.
 */
function apple_news_article_metadata($article_id, $article_revision_id = NULL) {
  $query = db_select('apple_news_published_entities', 'e')->fields('e');
  $query->condition('article_id', $article_id);
  if (!empty($article_revision_id)) {
    $query->condition('article_revision_id', $article_revision_id);
  }
  $post = $query->execute()
    ->fetchObject();
  $metadata = unserialize($post->metadata);
  if (!empty($metadata)) {
    if (!is_array($metadata)) {
      return unserialize($metadata);
    }
    else {
      return $metadata;
    }
  }
  return FALSE;
}
