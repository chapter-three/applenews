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
  $msg = 'Could not save export %machine_name defined by %module.';
  $var = array(
    '%machine_name' => $export->getMachineName(),
    '%module' => $export->getModule(),
  );
  try {
    $update = db_select('apple_news_status', 's')
      ->condition('s.module', $export->getModule())
      ->condition('s.machine_name', $export->getMachineName())
      ->countQuery()->execute()->fetchField();
    if ($update) {
      $success = db_update('apple_news_status')
        ->fields(array(
          'enabled' => (int) $export->enabled,
          'highwater' => $export->highwater,
          'settings' => serialize($export->settings),
        ))
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
        ->fields(array(
          'module' => $export->getModule(),
          'machine_name' => $export->getMachineName(),
          'enabled' => (int) $export->enabled,
          'highwater' => $export->highwater,
          'settings' => serialize($export->settings),
        ))
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
    $msg = 'No export %machine_name defined by %module.';
    $var = array(
      '%machine_name' => $machine_name,
      '%module' => $module,
    );
    watchdog('apple_news', $msg, $var, WATCHDOG_ERROR);
    return NULL;
  }
  $info = $exports[$key];
  if (!class_exists($info['class'])) {
    $msg = 'No export class %class found.';
    $var = array('%class' => $info['class']);
    watchdog('apple_news', $msg, $var, WATCHDOG_ERROR);
    return NULL;
  }
  try {
    if (!isset($info['arguments']) || !is_array($info['arguments'])) {
      $info['arguments'] = array();
    }
    if (isset($info['#status'])) {
      return new $info['class']($module, $machine_name, $info['arguments'],
        $info['#status']['enabled'], $info['#status']['highwater'],
        $info['#status']['settings']);
    }
    return new $info['class']($module, $machine_name, $info['arguments']);
  }
  catch (Exception $e) {
    $msg = 'Export class %class could not be constructed: %message';
    $var = array(
      '%class' => $info['class'],
      '%message' => $e->getMessage(),
    );
    watchdog('apple_news', $msg, $var, WATCHDOG_ERROR);
    return NULL;
  }
}

/**
 * Gets registered exports.
 */
function apple_news_exports($reset = NULL) {
  static $exports = array();
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
      $args = array(
        '%machine_name' => $row['machine_name'],
        '%module' => $row['module'],
      );
      watchdog('apple_news', $msg, $args, WATCHDOG_INFO);
      continue;
    }
    // Validate class.
    $class_name = $exports[$key]['class'];
    if (!class_exists($class_name)) {
      $msg = 'Class %class does not exist.';
      $args = array('%class' => $class_name);
      watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
      continue;
    }
    $reflect = new ReflectionClass($class_name);
    if ($reflect->isAbstract() || !$reflect->isSubclassOf('AppleNewsExport')) {
      $msg = 'Class %class is not a valid concrete export class';
      $args = array('%class' => $class_name);
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
  $implementations = array();
  foreach (module_implements('apple_news_api') as $module) {
    $function = $module . '_apple_news_api';
    $info = $function();
    if (!isset($info['api']) || $info['api'] != APPLE_NEWS_API_VERSION) {
      $msg = '%function supports Apple News API version %modversion, Apple News module API version is %version - migration support not loaded.';
      $args = array(
        '%function' => $function,
        '%modversion' => $info['api'],
        '%version' => APPLE_NEWS_API_VERSION,
      );
      watchdog('apple_news', $msg, $args, WATCHDOG_NOTICE);
      continue;
    }
    foreach ($info['exports'] as $machine_name => $export) {
      if (empty($export['class'])) {
        $msg = 'Export %machine_name defined by %module is missing required class';
        $args = array(
          '%machine_name' => $machine_name,
          '%module' => $module,
        );
        watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
        continue;
      }
      if (!preg_match('/^[a-zA-Z0-9-_]+$/', $machine_name)) {
        $msg = 'Export machine name "%machine_name" defined by %module must match /^[a-zA-Z0-9-_]$/';
        $args = array(
          '%machine_name' => $machine_name,
          '%module' => $module,
        );
        watchdog('apple_news', $msg, $args, WATCHDOG_WARNING);
        continue;
      }
      $key = "${module}::${machine_name}";
      $implementations[$key] = array_merge(array(
          'name' => '',
          'description' => '',
          'arguments' => array(),
        ), $export, array(
          'module' => $module,
          'machine_name' => $machine_name,
        ));
    }
  }

  // Allow modules to alter the migration information.
  drupal_alter('apple_news_api', $implementations);

  return $implementations;
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
        // Initialize array for entity type.
        if (!isset($entity_types[$entity_type])) {
          $entity_types[$entity_type] = [];
        }
        // Build an array of entity types and bundle names.
        if (!in_array($wrapper->getBundle(), $entity_types[$entity_type]) || !isset($entity_types[$entity_type])) {
          $entity_types[$entity_type][] = $wrapper->getBundle();
        }
      }
    }
  }
  return $entity_types;
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

  $config_fields = array(
    'apple_news_variable_api_endpoint',
    'apple_news_variable_api_key',
    'apple_news_variable_api_secret',
    'apple_news_variable_api_channels',
  );

  return $config_fields;

}

/**
 * Cleanup database tables.
 * @param (string) $channel_id
 *    Apple News channel ID.
 */
function apple_news_cleanup($channel_id) {

  // Delete settings when channel ID removed.
  $result = db_select('apple_news_status', 's')
    ->fields('s')
    ->execute();
  foreach ($result as $row) {
    $settings = unserialize($row->settings);
    unset($settings['channels'][$channel_id]);
    db_update('apple_news_status')
      ->fields(array(
        'settings' => serialize($settings),
      ))
      ->condition('module', $row->module)
      ->condition('machine_name', $row->machine_name)
      ->execute();

  }

}
