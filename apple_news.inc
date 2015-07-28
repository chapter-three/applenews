<?php

/**
 * @file
 * Helper functions.
 */

/**
 * Implements OBJECT_save().
 *
 * @param AppleNewsExport $export
 *   Object as returned by apple_news_export_load() or apple_news_get_export().
 *
 * @return bool
 *   Success.
 */
function apple_news_export_save($export) {
  $transaction = db_transaction();
  $msg = t('Could not save export %machine_name defined by %module.');
  $var = [
    '%machine_name' => $export->getMachineName(),
    '%module' => $export->getModule(),
  ];
  try {
    $update = db_select('apple_news_status', 's')
      ->condition('s.module', $export->getModule())
      ->condition('s.machine_name', $export->getMachineName())
      ->countQuery()->execute()->fetchField();
    if ($update) {
      $success = db_update('apple_news_status')
        ->fields(
          [
            'enabled' => (int) $export->enabled,
            'highwater' => $export->highwater,
            'settings' => serialize($export->settings),
          ]
        )
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
        ->fields(
          [
            'module' => $export->getModule(),
            'machine_name' => $export->getMachineName(),
            'enabled' => (int) $export->enabled,
            'highwater' => $export->highwater,
            'settings' => serialize($export->settings),
          ]
        )
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
 * Defines an export object id.
 */

/**
 * Gets an export object.
 *
 * @param string $module
 *   Module machine name.
 * @param string $machine_name
 *   Export machine name.
 *
 * @return AppleNewsExport
 *   The instantiated class configured for the export.
 *
 * @see apple_news_exports()
 */
function apple_news_get_export($module, $machine_name) {
  $exports = apple_news_exports();
  $key = "${module}::${machine_name}";
  if (empty($exports[$key])) {
    $msg = t('No export %machine_name defined by %module.');
    $var = [
      '%machine_name' => $machine_name,
      '%module' => $module,
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
  try {
    if (!isset($info['arguments']) || !is_array($info['arguments'])) {
      $info['arguments'] = [];
    }
    if (isset($info['#status'])) {
      return new $info['class']($module, $machine_name, $info['arguments'],
        $info['#status']['enabled'], $info['#status']['highwater'],
        $info['#status']['settings']);
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
  static $exports = [];
  if (!empty($exports) && empty($reset)) {
    return $exports;
  }

  $exports = apple_news_get_module_apis();

  $result = db_select('apple_news_status', 's')
    ->fields('s')
    ->execute();
  while ($row = $result->fetchAssoc()) {
    $key = $row['module'] . '::' . $row['machine_name'];
    // Module still enabled and export still defined.
    if (!isset($exports[$key])) {
      $msg = 'Export %machine_name defined by %module no longer defined.';
      $args = [
        '%machine_name' => $row['machine_name'],
        '%module' => $row['module'],
      ];
      watchdog('apple_news', $msg, $args, WATCHDOG_INFO);
      continue;
    }
    // Validate class.
    $class_name = $exports[$key]['class'];
    if (!class_exists($class_name)) {
      $msg = 'Class %class does not exist.';
      $args = [
        '%class' => $class_name
      ];
      watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
      continue;
    }
    $reflect = new ReflectionClass($class_name);
    if ($reflect->isAbstract() || !$reflect->isSubclassOf('AppleNewsExport')) {
      $msg = 'Class %class is not a valid concrete export class';
      $args = [
        '%class' => $class_name
      ];
      watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
      continue;
    }
    // Good to go.
    $row['settings'] = unserialize($row['settings']);
    $exports[$key]['#status'] = $row;
  }

  return $exports;
}

/**
 * Get a list of exports supporting the current export API.
 */
function apple_news_get_module_apis() {
  $implementations = [];
  foreach (module_implements('apple_news_api') as $module) {
    $function = $module . '_apple_news_api';
    $info = $function();
    if (!isset($info['api']) || $info['api'] != APPLE_NEWS_API_VERSION) {
      $msg = t('%function supports Apple News API version %modversion, Apple News module API version is %version - migration support not loaded.');
      $args = [
        '%function' => $function,
        '%modversion' => $info['api'],
        '%version' => APPLE_NEWS_API_VERSION,
      ];
      watchdog('apple_news', $msg, $args, WATCHDOG_NOTICE);
      continue;
    }
    foreach ($info['exports'] as $machine_name => $export) {
      if (empty($export['class'])) {
        $msg = t('Export %machine_name defined by %module is missing required class');
        $args = [
          '%machine_name' => $machine_name,
          '%module' => $module,
        ];
        watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
        continue;
      }
      if (!preg_match('/^[a-zA-Z0-9-_]+$/', $machine_name)) {
        $msg = t('Export machine name "%machine_name" defined by %module must match /^[a-zA-Z0-9-_]$/');
        $args = [
          '%machine_name' => $machine_name,
          '%module' => $module,
        ];
        watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
        continue;
      }
      $key = "${module}::${machine_name}";
      $implementations[$key] = array_merge(
        [
          'name' => '',
          'description' => '',
          'arguments' => [],
        ], 
        $export,
        [
          'module' => $module,
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
      'entity_type' => $entity_type,
      'entity_id' => $entity_wrapper->getIdentifier(),
      // Need to work on revision support.
      'revision_id' => $entity_wrapper->getIdentifier(),
      'article_id' => $response->data->id,
      'article_revision_id' => $response->data->revision,
      'channel_id' => $channel_id,
      'postdate' => REQUEST_TIME,
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
    $metadata = !empty($data['metadata']) ? json_decode($data['metadata']) : [];
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
            'article_id' => $response->data->id,
            'article_revision_id' => $response->data->revision,
            'postdate' => REQUEST_TIME,
          ]
        )
        ->condition('entity_id', $entity_wrapper->getIdentifier())
        ->condition('article_id', $article->article_id)
        ->condition('article_revision_id', $article->article_revision_id)
        ->condition('channel_id', $channel_id)
        ->execute();
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
 * Get an array of entity types and bundle names
 * that are available for Apple News export.
 *
 * @return array
 */
function apple_news_get_entity_types() {
  $entity_types = [];
  // Loop through all exports and build an array so we can alter entity forms
  // and add vertical tabs for individual Apple News publishing options.
  foreach (apple_news_exports() as $info) {
    // Load export object.
    $export = apple_news_get_export($info['module'], $info['machine_name']);
    $result = $export->query()->execute();
    foreach (array_keys($result) as $entity_type) {
      foreach ($result[$entity_type] as $entity_id => $object) {
        // Load entity and pass it to entity wrapper.
        $entity = entity_load_single($entity_type, $entity_id);
        $wrapper = entity_metadata_wrapper($entity_type, $entity);
        // Build an array of entity types and bundle names.
        $entity_types[$entity_type][$wrapper->getBundle()] = $info;
      }
    }
  }
  return $entity_types;
}

/**
 * Perform operations on an entity.
 *
 * @param $op string
 *   Entity operation (insert/update/delete)
 * @param $channel_id string
 *   Apple News channel ID.
 * @param $entity object
 *   Entity object.
 * @param $type string
 *   Entity type.
 */
function apple_news_op($op, $channel_id, $entity, $type, array $settings = []) {
  // Get all possible entity types and bundles available for export.
  $entity_types = apple_news_get_entity_types();
  foreach ($entity_types as $entity_type => $export) {
    foreach ($export as $bundle => $info) {
      if ($entity_type == $type) {
        $result = FALSE;
        // Export object.
        $export = apple_news_get_export($info['module'], $info['machine_name']);
        // Entity object wrapper.
        $wrapper = entity_metadata_wrapper($entity_type, $entity);
        // Generate Apple News Document formatted JSON string.
        $document = $export->export($entity);
        if (count($settings)) {
          // Merge export settings with custom fields from the entity add/edit form.
          // @see apple_news_form_alter().
          $export->settings = array_merge($export->settings, $settings);
        }
        $data = [
          'json' => $document->__toString(),
          'files' => $export->assets($entity), // Get entity assets.
          'metadata' => '',
        ];
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
    }
  }
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
function apple_news_channels() {
  // Add Post to Apple News checkbox.
  $channels = [];
  $api_channels = variable_get('apple_news_variable_api_channels', []);
  foreach ($api_channels as $channel_id) {
    $channel = apple_news_channel($channel_id);
    $channels[$channel->id] = filter_xss($channel->name);
  }
  return $channels;
}

/**
 * Get Apple News sections.
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
 * Get Apple News channels.
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
 * @param $entity obejct
 *   Entity object.
 * @param $type string
 *   Entity type.
 *
 * @return object
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
 * Check if the entity already published to Apple News
 *
 * @param $channel_id string
 *   Apple News channel ID
 * @param $entity_wrapper obejct
 *   Entity wrapper object.
 * @param $type string
 *   Entity type.
 *
 * @return Export log record or FALSE if not results.
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
 * @param $entity_wrapper obejct
 *   Entity wrapper object.
 * @param $type string
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
